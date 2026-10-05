#pragma once
#include "hittable.hpp"
#include "interval.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "rtweekend.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <random>
#include <thread>

struct camera_settings {
  int image_width = 400;
  double aspect_ratio = 1; // divide in caller
  double vfov = 90;        // vertical field of view [deg]
  point3 look_from = point3(0, 0, 0);
  point3 look_at = point3(0, 0, -1);
  vec3 v_up = vec3(0, 1, 0);
  color background_color = color(0.7, 0.8, 1.0);
  int seed = 1337;
  int samples_per_pixel = 100;
  int max_depth = 10;
  double defocus_angle = 0;
  double focus_dist = 10;
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
    background_color = cs.background_color;
    assert(cs.max_depth > 0);
    max_depth = cs.max_depth;

    partition = 1.0 / samples_per_pixel;

    assert(interval(0, 180).surrounds(cs.vfov));

    // focus
    assert(cs.defocus_angle >= 0 && cs.defocus_angle < 120);
    assert(cs.focus_dist > 0);
    defocus_angle = cs.defocus_angle;
    // Orthonormal basis in camera coordinates
    assert((cs.look_from - cs.look_at).length() > 0 + 1e-6);
    vec3 w = cs.look_from - cs.look_at;
    double focal_length = w.length();
    w /= focal_length;
    vec3 u = cross(cs.v_up, w);
    double u_len = u.length();
    assert(u_len > 0 + 1e-6);
    u /= u_len;
    vec3 v = cross(w, u);

    double vp_height = 2 * std::tan(deg_to_rad(cs.vfov) / 2) * cs.focus_dist;
    double viewport_width = vp_height * (double(image_width) / image_height);

    vec3 vu = viewport_width * u; // viewport horizontal
    vec3 vv = -vp_height * v;     // viewport vertical

    // defocus disk
    double rad = cs.focus_dist * std::tan(deg_to_rad(defocus_angle) / 2);
    u_defocus = u * rad;
    v_defocus = v * rad;

    du = vu / image_width;
    dv = vv / image_height;

    C = cs.look_from;

    Q_ul = C - cs.focus_dist * w - vu / 2 - vv / 2;
  }

  void render(std::ostream &stream, const hittable &world) const {
    stream << "P3 " << image_width << " " << image_height << " " << 255 << "\n";
    auto color_buf = std::vector<color>(image_height * image_width);
    std::atomic<int> next{0};
    std::atomic<int> completed{0};
    auto worker = [&] {
      while (true) {
        const int j = next.fetch_add(1);
        std::seed_seq seq{seed, j};
        std::mt19937 rng(seq);
        if (j >= image_height) {
          break;
        }
        for (int i = 0; i < image_width; i++) {
          color c = color(0, 0, 0);
          for (int n = 0; n < samples_per_pixel; n++) {
            ray r = get_ray(i, j, rng);
            c += ray_color(r, world, max_depth, rng);
          }
          color_buf[size_t(image_width * j + i)] = c * partition;
        }
        std::clog << "\rscanlines remaining: "
                  << (image_height - (completed.fetch_add(1) + 1)) << ' '
                  << std::flush;
      }
    };

    { // scope so that threads join before write
      std::vector<std::jthread> threads;
      const unsigned n = std::max(1u, std::thread::hardware_concurrency());
      for (unsigned t = 0; t < n; t++)
        threads.emplace_back(worker);
    }
    std::clog << "\rDone                              \n";
    for (const auto &c : color_buf)
      write_color(stream, c);
  }

private:
  int image_width;
  int image_height;
  int seed;
  color background_color;
  int samples_per_pixel;
  double partition;
  int max_depth;
  double defocus_angle;
  vec3 du; // horizontal pixel width
  vec3 dv; // vertical pixel width
  vec3 u_defocus;
  vec3 v_defocus;
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
      color emitted = rec.mat->emitted(rec.u, rec.v, rec.p);

      if (rec.mat->scatter(r, rec, attenuation, scattered, rng)) {
        return emitted +
               attenuation * ray_color(scattered, world, depth - 1, rng);
      }
      return emitted;
    }
    return background_color;
  }

  ray get_ray(int i, int j, std::mt19937 &rng) const {
    vec3 p_ij = pixel_center(i, j);
    double u_offset = random_double(rng, -0.5, 0.5);
    double v_offset = random_double(rng, -0.5, 0.5);
    point3 p = p_ij + u_offset * du + v_offset * dv;

    point3 ray_origin = C;
    if (defocus_angle > 0) {
      point3 ru = random_in_unit_disk(rng);
      ray_origin += ru.x() * u_defocus + ru.y() * v_defocus;
    }
    return ray(ray_origin, p - ray_origin, random_double(rng, 0, 1));
  }
};