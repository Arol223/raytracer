#include "camera.hpp"
#include "hittable.hpp"
#include "material.hpp"
#include "rtweekend.hpp"
#include "vec3.hpp"
#include <cmath>
#include <iostream>
#include <memory>

hittable_list scene_generation(std::uint32_t seed = 42) {
  hittable_list world;
  std::mt19937 rng(seed); // separate engine from the renderer's

  auto ground_material = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
  world.add(
      std::make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

  // Grid of small spheres with random positions and materials
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      const double choose_mat = random_double(rng, 0, 1);
      const double x = a + 0.9 * random_double(rng, 0, 1);
      const double z = b + 0.9 * random_double(rng, 0, 1);
      const point3 center(x, 0.2, z);

      // Keep clear of the big metal sphere
      if ((center - point3(4, 0.2, 0)).length() <= 0.9)
        continue;

      std::shared_ptr<material> sphere_material;
      bool diffuse = false;
      if (choose_mat < 0.8) { // diffuse
        const color c1 = random_vec(rng, 0, 1);
        const color c2 = random_vec(rng, 0, 1);
        sphere_material = std::make_shared<lambertian>(c1 * c2);
        diffuse = true;
      } else if (choose_mat < 0.95) { // metal
        const color albedo = random_vec(rng, 0.5, 1);
        const double fuzz = random_double(rng, 0, 0.5);
        sphere_material = std::make_shared<metal>(albedo, fuzz);
      } else { // glass
        sphere_material = std::make_shared<dielectric>(1.5);
      }
      if (diffuse) {
        world.add(std::make_shared<sphere>(
            center, center + point3(0, random_double(rng, 0, 0.1), 0), 0.2,
            sphere_material));
      } else {
        world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
      }
    }
  }

  // Three large feature spheres
  world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0,
                                     std::make_shared<dielectric>(1.5)));
  world.add(std::make_shared<sphere>(
      point3(-4, 1, 0), 1.0,
      std::make_shared<lambertian>(color(0.4, 0.2, 0.1))));
  world.add(std::make_shared<sphere>(
      point3(4, 1, 0), 1.0,
      std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0)));

  return world;
}

int main() {
  // aspect ratio 16:9
  camera_settings cs = camera_settings();
  cs.aspect_ratio = 16.0 / 9.0;
  cs.image_width = 400;
  cs.look_from = point3(13, 2, 3);
  cs.look_at = point3(0, 0, 0);
  cs.v_up = vec3(0, 1, 0);
  cs.vfov = 20;
  cs.max_depth = 50;
  cs.samples_per_pixel = 100;
  cs.focus_dist = 10;
  cs.defocus_angle = 0.6;
  camera c = camera(cs);
  hittable_list world = scene_generation();

  /*
  hittable_list world = hittable_list();
  // material
  auto lamb1 = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
  auto lamb2 = std::make_shared<lambertian>(lambertian(color(0.1, 0.2, 0.5)));
  auto metal1 = std::make_shared<metal>(color(0.8, 0.8, 0.8), 0.0);
  auto metal2 = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);
  auto glass = std::make_shared<dielectric>(1.5);
  auto air_bubble = std::make_shared<dielectric>(1 / 1.5);

  world.add(std::make_shared<sphere>(point3(0, -100.5, -1), 100, lamb1));
  world.add(std::make_shared<sphere>(point3(0, 0, -1.2), 0.5, lamb2));
  world.add(std::make_shared<sphere>(point3(-1, 0, -1), 0.5, glass));
  world.add(std::make_shared<sphere>(point3(-1, 0, -1), 0.4, air_bubble));
  world.add(std::make_shared<sphere>(point3(1, 0, -1), 0.5, metal2));
 */
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
