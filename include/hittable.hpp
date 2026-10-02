#pragma once
#include "interval.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
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
};

class sphere : public hittable {
public:
  sphere(const point3 &center, double radius,
         const std::shared_ptr<material> mat)
      : center(center), radius(std::fmax(0, radius)), mat(mat) {
    assert(mat != nullptr);
  }

  bool hit(const ray &r, const interval &ray_t,
           hit_record &rec) const override {
    vec3 oc = center - r.o();
    double h = dot(r.d(), oc);
    double c = oc.length_squared() - radius * radius;

    double disc = h * h - c;
    if (disc < 0)
      return false;
    double disc_root = std::sqrt(disc);

    for (double t : {h - disc_root, h + disc_root})
      if (ray_t.surrounds(t)) {
        rec.t = t;
        rec.p = r.at(t);
        rec.set_face_normal(r, (rec.p - center) / radius);
        rec.mat = mat;
        return true;
      }
    return false;
  }

private:
  point3 center;
  double radius;
  std::shared_ptr<material> mat;
};

class hittable_list : public hittable {
public:
  hittable_list() = default;
  explicit hittable_list(std::shared_ptr<hittable> h) { add(h); }

  void add(std::shared_ptr<hittable> h) { hit_list.push_back(std::move(h)); }
  void clear() { hit_list.clear(); }

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

private:
  std::vector<std::shared_ptr<hittable>> hit_list;
};