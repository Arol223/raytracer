#pragma once
#include "aabb.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

class material; // forward declaration

struct hit_record {
  static constexpr double tol = 1e-6;
  point3 p;
  vec3 normal;
  double t = 0;
  bool front_face = false;
  std::shared_ptr<material> mat;
  double u, v; // surface coordinates

  void set_face_normal(const ray &r, const vec3 &outward_normal) {
    assert(std::abs(outward_normal.length_squared() - 1) < tol);
    front_face = dot(r.d(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

class hittable {
public:
  virtual ~hittable() = default;

  virtual bool hit(const ray &r, const interval &ray_t,
                   hit_record &rec) const = 0;
  virtual aabb bounding_box() const = 0;
};

class sphere : public hittable {
public:
  sphere(const point3 &center, double radius, // Stationary constructor
         const std::shared_ptr<material> mat)
      : center(center), rad(std::fmax(0, radius)), mat(mat) {
    assert(mat != nullptr);
    displacement = vec3(0, 0, 0);
    vec3 p = vec3(rad);
    bbox = aabb(center - p, center + p);
  }
  sphere(const point3 &center1, const point3 &center2, double radius,
         const std::shared_ptr<material> mat)
      : center(center1), displacement(center2 - center1),
        rad(std::fmax(0, radius)), mat(mat) {
    assert(mat != nullptr);
    vec3 p = vec3(rad);
    bbox = aabb(aabb(center1 - p, center1 + p), aabb(center2 - p, center2 + p));
  }

  bool hit(const ray &r, const interval &ray_t,
           hit_record &rec) const override {
    point3 new_center = current_center(r.time());
    vec3 oc = new_center - r.o();
    double h = dot(r.d(), oc);
    double c = oc.length_squared() - rad * rad;

    double disc = h * h - c;
    if (disc < 0)
      return false;
    double disc_root = std::sqrt(disc);

    for (double t : {h - disc_root, h + disc_root})
      if (ray_t.surrounds(t)) {
        rec.t = t;
        rec.p = r.at(t);
        vec3 outward_normal = (rec.p - new_center) / rad;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;
        get_uv(outward_normal, rec.u, rec.v);
        return true;
      }
    return false;
  }

  point3 current_center(double t) const { return center + displacement * t; }

  aabb bounding_box() const override { return bbox; }

private:
  point3 center;
  vec3 displacement;
  double rad;
  std::shared_ptr<material> mat;
  aabb bbox; // Bounding Box

  static void get_uv(const point3 &p, double &u, double &v) {
    // uses y up convention, not traditional spherical coords from physics
    double theta = std::acos(-p.y());
    double phi = std::atan2(-p.z(), p.x()) + pi;
    u = phi / (2 * pi);
    v = theta / pi;
  }
};

class hittable_list : public hittable {
public:
  hittable_list() = default;
  explicit hittable_list(std::shared_ptr<hittable> h) { add(h); }

  void add(std::shared_ptr<hittable> h) {
    bbox = aabb(bbox, h->bounding_box());
    hit_list.push_back(std::move(h));
  }
  void clear() {
    bbox = aabb();
    hit_list.clear();
  }

  bool hit(const ray &r, const interval &ray_t,
           hit_record &rec) const override {
    double closest_so_far = ray_t.max;
    bool found_hit = false;
    for (const auto &h : hit_list) {
      hit_record temp_rec;

      if (h->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
        rec = temp_rec;
        found_hit = true;
        closest_so_far = rec.t;
      }
    }
    return found_hit;
  }

  aabb bounding_box() const override { return bbox; }

  const auto &get_hit_list() const { return hit_list; }

private:
  std::vector<std::shared_ptr<hittable>> hit_list;
  aabb bbox;
};

class bvh_node : public hittable {
public:
  explicit bvh_node(std::vector<std::shared_ptr<hittable>> objects)
      : bvh_node(objects, 0, objects.size()) {}
  bvh_node(std::vector<std::shared_ptr<hittable>> &objects, std::size_t start,
           std::size_t end) {
    assert(end > start);
    aabb box = aabb();
    for (auto i = start; i < end; i++) {
      box = aabb(box, objects[i]->bounding_box());
    }
    if (end - start == 1) {
      left = objects[start];
      right = objects[start];
    } else if (end - start == 2) {
      left = objects[start];
      right = objects[start + 1];
    } else {
      int axis = box.longest_axis();
      std::sort(objects.begin() + start, objects.begin() + end,
                [axis](const auto &a, const auto &b) {
                  return a->bounding_box().axis_interval(axis).min <
                         b->bounding_box().axis_interval(axis).min;
                });
      size_t mid = start + (end - start) / 2;
      left = std::make_shared<bvh_node>(objects, start, mid);
      right = std::make_shared<bvh_node>(objects, mid, end);
    }
    bbox = box;
  }
  bool hit(const ray &r, const interval &ray_t,
           hit_record &rec) const override {
    if (!bbox.hit(r, ray_t)) {
      return false;
    }
    bool hit_left = left->hit(r, ray_t, rec);
    bool hit_right =
        right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);
    return hit_left || hit_right;
  }

  aabb bounding_box() const override { return bbox; }

private:
  std::shared_ptr<hittable> left, right;

  aabb bbox;
};
