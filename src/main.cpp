#include "camera.hpp"
#include "hittable.hpp"
#include "vec3.hpp"
#include <cmath>
#include <iostream>
#include <memory>

int main() {
  // aspect ratio 16:9
  camera_settings cs = camera_settings();
  cs.aspect_ratio = 16.0 / 9.0;
  camera c = camera(cs);

  hittable_list world = hittable_list();
  world.add(std::make_shared<sphere>(point3(0, -100.5, -1), 100));
  world.add(std::make_shared<sphere>(point3(0, 0, -1), 0.5));

  c.render(std::cout, world);

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

    */
  return 0;
}
