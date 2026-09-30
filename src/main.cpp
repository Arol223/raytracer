#include "hittable.hpp"
#include "ray.hpp"
#include "rtweekend.hpp"
#include "vec3.hpp"
#include <cmath>
#include <iostream>
#include <memory>

namespace {

color ray_color(ray r, const hittable &hittables) {
  hit_record rec;

  if (hittables.hit(r, interval(0, infinity), rec)) {

    return 0.5 *
           color(rec.normal.x() + 1, rec.normal.y() + 1, rec.normal.z() + 1);
  }
  color white = color(1.0, 1.0, 1.0);

  double a = 0.5 * (r.d()[1] + 1);

  return (1 - a) * white + a * color(0.5, 0.7, 1.0);
}
} // namespace

int main() {
  // aspect ratio 16:9
  int image_width = 400;
  int image_height = image_width / (16.0 / 9.0);

  double vp_height = 2.0; // viewport
  double fl = 1.0;
  double vp_width = vp_height * (double(image_width) / image_height);

  vec3 vu(vp_width, 0, 0);   // viewport horizontal edge vector
  vec3 vv(0, -vp_height, 0); // viewport vertical edge vector

  vec3 du = vu / image_width;
  vec3 dv = vv / image_height;

  point3 C = point3(0, 0, 0); // Camera center

  point3 Q_ul = C - point3(0, 0, fl) - vu / 2 - vv / 2;

  auto pixel_center = [Q_ul, du, dv](int i, int j) {
    return Q_ul + 0.5 * (du + dv) + i * du + j * dv;
  };

  hittable_list world = hittable_list();
  world.add(std::make_shared<sphere>(point3(0, -100.5, -1), 100));
  world.add(std::make_shared<sphere>(point3(0, 0, -1), 0.5));

  std::cout << "P3" << " " << image_width << " " << image_height << " "
            << "255\n";
  for (int j = 0; j < image_height; j++) {
    for (int i = 0; i < image_width; i++) {
      vec3 p_ij = pixel_center(i, j);
      ray r = ray(C, p_ij - C);
      color c = ray_color(r, world);
      write_color(std::cout, c);
    }
  }

  /*
    std::cout << "P3" << " " << "256" << " " << "256" << " " << "255\n";

    const float b = 0;
    const float scale = 255.999;
    const int image_width = 256;
    const int image_height = 256;
    for (int j = 0; j < 256; j++) {
      std::clog << "\rscanlines remaining: " << (256 - j) << ' ' << std::flush;
      for (int i = 0; i < 256; i++) {
        float r = i / (image_width - 1.0f);
        float g = j / (image_height - 1.0f);
        std::cout << int(r * scale) << " " << int(g * scale) << " " << int(b)
                  << "\n";
      }
    }
    std::clog << "\rDone                              \n";
    */
  return 0;
}
