#pragma once
#include "hittable.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "rtweekend.hpp"
#include "vec3.hpp"
#include <cassert>
#include <cmath>
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

  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation,
               ray &scattered, std::mt19937 &rng) const override {
    vec3 scatter_dir = rec.normal + random_unit_vector(rng);
    if (scatter_dir.near_zero())
      scatter_dir = rec.normal;
    scattered = ray(rec.p, scatter_dir, r_in.time());
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
    scattered = ray(rec.p, scatter_dir, r_in.time());
    attenuation = albedo;
    return dot(scatter_dir, rec.normal) > 0;
  }

private:
  color albedo;
  double fuzz;
};

class dielectric : public material {
public:
  explicit dielectric(double ref_ind) : refraction_index(ref_ind) {
    assert(ref_ind > 0);
  }

  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation,
               ray &scattered, std::mt19937 &rng) const override {
    double eff_ratio = rec.front_face ? 1 / refraction_index : refraction_index;
    double cos_theta = std::min(-dot(r_in.d(), rec.normal), 1.0);
    double sin_theta = std::sqrt(1 - cos_theta * cos_theta);

    bool cannot_refract =
        eff_ratio * sin_theta > 1; // total internal reflection

    vec3 scatter_dir = (cannot_refract ||
                        reflectance(cos_theta, eff_ratio) > random_double(rng))
                           ? reflect(r_in.d(), rec.normal)
                           : refract(r_in.d(), rec.normal, eff_ratio);

    attenuation = color(1, 1, 1);
    scattered = ray(rec.p, scatter_dir, r_in.time());
    return true;
  }

private:
  double refraction_index;
  static double reflectance(double cosine, double eff_ratio) {

    double r0 = (1 - eff_ratio) / (1 + eff_ratio);
    r0 *= r0;
    return r0 + (1 - r0) * std::pow(1 - cosine, 5);
  }
};
