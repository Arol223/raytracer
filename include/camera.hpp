#pragma once
#include "hittable.hpp"
#include "interval.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <ostream>
#include <random>

struct camera_settings {
  int image_width = 400;
  double aspect_ratio = 1; // divide in caller
  double viewport_height = 2.0;
  double focal_length = 1.0;
  int seed = 1337;
  int samples_per_pixel = 100;
  int max_depth = 10;
  point3 camera_center = point3(0, 0, 0);
};

class camera {
public:
  camera(const camera_settings &cs) {
    assert(cs.image_width >= 1);
    image_width = cs.image_width;
    assert(cs.aspect_ratio > 0);
    image_height = std::max(int(image_width / cs.aspect_ratio), 1);
    seed = cs.seed;
    assert(cs.samples_per_pixel > 0);
    samples_per_pixel = cs.samples_per_pixel;

    assert(cs.max_depth > 0);
    max_depth = cs.max_depth;

    partition = 1.0 / samples_per_pixel;
    double viewport_width =
        cs.viewport_height * (double(image_width) / image_height);

    vec3 u = vec3(viewport_width, 0, 0);
    vec3 v = vec3(0, -cs.viewport_height, 0);

    du = u / image_width;
    dv = v / image_height;

    C = cs.camera_center;

    Q_ul = C - point3(0, 0, cs.focal_length) - u / 2 - v / 2;
  }

  void render(std::ostream &stream, const hittable &world) const {
    stream << "P3 " << image_width << " " << image_height << " " << 255 << "\n";
    auto rng = std::mt19937(seed);
    for (int j = 0; j < image_height; j++) {
      std::clog << "\rscanlines remaining: " << (image_height - j) << ' '
                << std::flush;
      for (int i = 0; i < image_width; i++) {
        color c = color(0, 0, 0);
        for (int n = 0; n < samples_per_pixel; n++) {
          ray r = get_ray(i, j, rng);
          c += ray_color(r, world, max_depth, rng);
        }
        write_color(stream, c * partition);
      }
    }
    std::clog << "\rDone                              \n";
  }

private:
  int image_width;
  int image_height;
  int seed;
  int samples_per_pixel;
  double partition;
  int max_depth;
  vec3 du;  // horizontal pixel width
  vec3 dv;  // vertical pixel width
  point3 C; // camera center
  point3 Q_ul;

  vec3 pixel_center(const int i, const int j) const {
    return Q_ul + 0.5 * (du + dv) + i * du + j * dv;
  }

  color ray_color(const ray &r, const hittable &world, const int depth,
                  std::mt19937 &rng) const {
    hit_record rec;

    if (depth <= 0)
      return color(0, 0, 0);

    if (world.hit(r, interval(0.001, infinity), rec)) {

      color attenuation = color();
      ray scattered = r;

      if (rec.mat->scatter(r, rec, attenuation, scattered, rng)) {
        return attenuation * ray_color(scattered, world, depth - 1, rng);
      }
      return color(0, 0, 0);
    }
    color white = color(1.0, 1.0, 1.0);

    double a = 0.5 * (r.d()[1] + 1);

    return (1 - a) * white + a * color(0.5, 0.7, 1.0);
  }

  ray get_ray(int i, int j, std::mt19937 &rng) const {
    vec3 p_ij = pixel_center(i, j);
    double u_offset = random_double(rng, -0.5, 0.5);
    double v_offset = random_double(rng, -0.5, 0.5);
    point3 p = p_ij + u_offset * du + v_offset * dv;
    return ray(C, p - C);
  }
};