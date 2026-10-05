#pragma once
#include "material.hpp"

class constant_medium : public hittable {
public:
  constant_medium(std::shared_ptr<hittable> boundary, double density,
                  std::shared_ptr<texture> tex)
      : boundary(std::move(boundary)), neg_inv_density(-1.0 / density),
        phase_function(std::make_shared<isotropic>(tex)) {
    assert(density > 0);
  }
  constant_medium(std::shared_ptr<hittable> boundary, double density, color tex)
      : boundary(std::move(boundary)), neg_inv_density(-1.0 / density),
        phase_function(std::make_shared<isotropic>(tex)) {
    assert(density > 0);
  }

  aabb bounding_box() const override { return boundary->bounding_box(); }

  bool hit(const ray &r, const interval &ray_t,
           hit_record &rec) const override {
    hit_record rec1 = hit_record();
    if (!boundary->hit(r, interval::universe, rec1)) {
      return false;
    }
    hit_record rec2 = hit_record();
    if (!boundary->hit(r, interval(rec1.t + 0.0001, infinity), rec2)) {
      return false;
    }
    rec1.t = std::max(rec1.t, ray_t.min);
    rec2.t = std::min(rec2.t, ray_t.max);
    if (rec1.t >= rec2.t) {
      return false;
    }
    rec1.t = std::max(rec1.t, 0.0);
    double ray_length = r.d().length();
    double dist_in_medium = (rec2.t - rec1.t) * ray_length;
    auto &rng = thread_rng();
    double hit_distance = neg_inv_density * std::log(random_double(rng));
    if (hit_distance > dist_in_medium) {
      return false;
    }
    rec.t = rec1.t + hit_distance / ray_length;
    rec.p = r.at(rec.t);
    rec.normal = vec3(1, 0, 0);
    rec.front_face = true;
    rec.u = rec.v = 0;
    rec.mat = phase_function;
    return true;
  }

private:
  std::shared_ptr<hittable> boundary;
  double neg_inv_density;
  std::shared_ptr<material> phase_function;
};