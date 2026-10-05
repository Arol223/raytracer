#pragma once
#include "aabb.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "rtweekend.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <array>
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
  double u, v = 0; // surface coordinates

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

class planar_shape : public hittable {
public:
  enum class kind { quad, triangle, ellipse };

  planar_shape(const point3 &Q, const vec3 &u, const vec3 &v,
               const std::shared_ptr<material> mat, kind shape = kind::quad)
      : corner(Q), u_edge(u), v_edge(v), mat(mat), shape(shape) {
    vec3 n = cross(u, v);
    double part = dot(n, n);
    assert(part > 0);
    surface_normal = n / std::sqrt(part);
    w = n / part;
    D = dot(corner, surface_normal);
    bbox = aabb(aabb(corner, corner + u + v), aabb(corner + u, corner + v));
  }

  bool hit(const ray &r, const interval &ray_t,
           hit_record &rec) const override {
    double nd = dot(surface_normal, r.d());
    if (std::abs(nd) < 1e-8) {
      return false;
    }
    double t = (D - dot(surface_normal, r.o())) / nd;
    if (!ray_t.contains(t)) {
      return false;
    }
    point3 P = r.at(t);
    point3 p = P - corner;
    double alpha = dot(w, cross(p, v_edge));
    double beta = dot(w, cross(u_edge, p));
    if (is_interior(alpha, beta)) {
      rec.u = alpha;
      rec.v = beta;
      rec.t = t;
      rec.p = P;
      rec.mat = mat;
      rec.set_face_normal(r, surface_normal);
      return true;
    }
    return false;
  }

  aabb bounding_box() const override { return bbox; }

private:
  point3 corner;
  vec3 u_edge;
  vec3 v_edge;
  std::shared_ptr<material> mat;
  kind shape;
  vec3 surface_normal;
  double D;
  aabb bbox; // bounding box
  vec3 w;

  bool is_interior(double alpha, double beta) const {
    switch (shape) {
    case kind::quad:
      return (interval(0, 1).contains(alpha) && interval(0, 1).contains(beta));
    case kind::triangle:
      return (alpha >= 0 && beta >= 0 && alpha + beta <= 1);
    case kind::ellipse:
      return ((alpha - 0.5) * (alpha - 0.5) + (beta - 0.5) * (beta - 0.5) <=
              0.25);
    }
    return false;
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
      std::size_t mid = start + (end - start) / 2;
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

inline std::shared_ptr<hittable_list> box(const point3 &a, const point3 &b,
                                          std::shared_ptr<material> mat) {
  point3 min(0, 0, 0);
  point3 max(0, 0, 0);

  for (int i = 0; i < 3; i++) {
    min[i] = std::min(a[i], b[i]);
    max[i] = std::max(a[i], b[i]);
  }

  vec3 dx = vec3(max.x() - min.x(), 0, 0);
  vec3 dy = vec3(0, max.y() - min.y(), 0);
  vec3 dz = vec3(0, 0, max.z() - min.z());

  auto sides = std::make_shared<hittable_list>();
  auto add = [&](const point3 &corner, const vec3 &u, const vec3 &v) {
    sides->add(std::make_shared<planar_shape>(corner, u, v, mat));
  };
  add(point3(min.x(), min.y(), max.z()), dx, dy);  // front (+z)
  add(point3(max.x(), min.y(), max.z()), -dz, dy); // right (+x)
  add(point3(max.x(), min.y(), min.z()), -dx, dy); // back (-z)
  add(point3(min.x(), min.y(), min.z()), dz, dy);  // left(-x)
  add(point3(min.x(), max.y(), max.z()), dx, -dz); // top(+y)
  add(point3(min.x(), min.y(), min.z()), dx, dz);  // bottom (-y)
  return sides;
}

class translate : public hittable {
public:
  translate(std::shared_ptr<hittable> obj, const vec3 &offs)
      : object(std::move(obj)), offset(offs),
        bbox(object->bounding_box() + offset) {}

  bool hit(const ray &r, const interval &ray_t,
           hit_record &rec) const override {
    ray offset_ray = ray(r.o() - offset, r.d(), r.time());

    if (object->hit(offset_ray, ray_t, rec)) {
      rec.p += offset;
      return true;
    }
    return false;
  }

  aabb bounding_box() const override { return bbox; }

private:
  std::shared_ptr<hittable> object;
  vec3 offset;
  aabb bbox;
};

class rotate : public hittable {
public:
  rotate(std::shared_ptr<hittable> obj, double theta)
      : object(std::move(obj)), sin_theta(std::sin(deg_to_rad(theta))),
        cos_theta(std::cos(deg_to_rad(theta))) {

    // Constructing rotated bounding box
    std::array<interval, 3> intervals;
    std::array<vec3, 8> corners = get_corners(object->bounding_box());
    for (int i = 0; i < 3; i++) {
      double min_axis = +infinity, max_axis = -infinity;
      for (vec3 corner : corners) {
        vec3 rotated = rot(corner);
        min_axis = std::min(min_axis, rotated[i]);
        max_axis = std::max(max_axis, rotated[i]);
      }
      intervals[i] = interval(min_axis, max_axis);
    }
    bbox = aabb(intervals[0], intervals[1], intervals[2]);
  }

  aabb bounding_box() const override { return bbox; }

  bool hit(const ray &r, const interval &ray_t,
           hit_record &rec) const override {
    ray rot_ray = ray(inv_rot(r.o()), inv_rot(r.d()), r.time());
    if (object->hit(rot_ray, ray_t, rec)) {
      rec.p = rot(rec.p);
      rec.normal = rot(rec.normal);
      return true;
    }
    return false;
  }

private:
  std::shared_ptr<hittable> object;
  double sin_theta;
  double cos_theta;
  aabb bbox;

  vec3 rot(const vec3 &u) const {
    return vec3(u.x() * cos_theta + u.z() * sin_theta, u.y(),
                -sin_theta * u.x() + cos_theta * u.z());
  }
  vec3 inv_rot(const vec3 &u) const {
    return vec3(cos_theta * u.x() - sin_theta * u.z(), u.y(),
                sin_theta * u.x() + cos_theta * u.z());
  }
};