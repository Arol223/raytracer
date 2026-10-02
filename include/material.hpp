#pragma once
#include "hittable.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include <cstdlib>
#include <random>

class material {
public:
  virtual ~material() = default;
  virtual bool scatter(const ray &r_in, const hit_record &rec,
                       color &attenuation, ray &scattered,
                       std::mt19937 &rng) const = 0;
};

class lambertian : public material {
public:
  explicit lambertian(color albedo) : alb(albedo) {}

  bool scatter(const ray & /*r_in*/, const hit_record &rec, color &attenuation,
               ray &scattered, std::mt19937 &rng) const override {
    vec3 scatter_dir = rec.normal + random_unit_vector(rng);
    if (scatter_dir.near_zero())
      scatter_dir = rec.normal;
    scattered = ray(rec.p, scatter_dir);
    attenuation = alb;
    return true;
  }

private:
  color alb;
};

class metal : public material {
public:
  explicit metal(color albedo, double fuzz)
      : albedo(albedo), fuzz(interval(0, 1).clamp(fuzz)) {}
  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation,
               ray &scattered, std::mt19937 &rng) const override {

    vec3 scatter_dir =
        reflect(r_in.d(), rec.normal) + fuzz * random_unit_vector(rng);
    scattered = ray(rec.p, scatter_dir);
    attenuation = albedo;
    return dot(scatter_dir, rec.normal) > 0;
  }

private:
  color albedo;
  double fuzz;
};
