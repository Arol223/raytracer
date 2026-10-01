#pragma once
#include "hittable.hpp"
#include "interval.hpp"
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
  int seed = 1337.0;
  int samples_per_pixel = 10;
  point3 camera_center = point3(0, 0, 0);
};

class camera {
public:
  camera(const camera_settings &cs) {
    assert(cs.image_width >= 1);
    image_width = cs.image_width;
    assert(cs.aspect_ratio > 0);
    image_height = std::max(int(image_width / cs.aspect_ratio), 1);
    double seed = cs.seed;
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
    std::mt19937 rng = std::mt19937(seed);
    stream << "P3 " << image_width << " " << image_height << " " << 255 << "\n";

    for (int j = 0; j < image_height; j++) {
      std::clog << "\rscanlines remaining: " << (image_height - j) << ' '
                << std::flush;
      for (int i = 0; i < image_width; i++) {
        vec3 p_ij = pixel_center(i, j);
        ray r = ray(C, p_ij - C);
        color c = ray_color(r, world);
        write_color(stream, c);
      }
    }
    std::clog << "\rDone                              \n";
  }

private:
  int image_width;
  int image_height;
  int seed;
  vec3 du;  // horizontal pixel width
  vec3 dv;  // vertical pixel width
  point3 C; // camera center
  point3 Q_ul;

  vec3 pixel_center(const int i, const int j) const {
    return Q_ul + 0.5 * (du + dv) + i * du + j * dv;
  }

  color ray_color(const ray &r, const hittable &hittables) const {
    hit_record rec;

    if (hittables.hit(r, interval(0, infinity), rec)) {

      return 0.5 *
             color(rec.normal.x() + 1, rec.normal.y() + 1, rec.normal.z() + 1);
    }
    color white = color(1.0, 1.0, 1.0);

    double a = 0.5 * (r.d()[1] + 1);

    return (1 - a) * white + a * color(0.5, 0.7, 1.0);
  }
};