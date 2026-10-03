#include "scenes.hpp"
#include "camera.hpp"
#include "hittable.hpp"
#include <cstdint>

void bouncing_spheres(std::uint32_t seed) {
  hittable_list world;

  std::mt19937 rng(seed);
  auto rnd = [&rng](double lo = 0.0, double hi = 1.0) {
    return std::uniform_real_distribution<double>(lo, hi)(rng);
  };

  auto checker = std::make_shared<checker_texture>(0.32, color(.2, .3, .1),
                                                   color(.9, .9, .9));
  world.add(std::make_shared<sphere>(point3(0, -1000, 0), 1000,
                                     std::make_shared<lambertian>(checker)));

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      double choose_mat = rnd();
      point3 center(a + 0.9 * rnd(), 0.2, b + 0.9 * rnd());

      if ((center - point3(4, 0.2, 0)).length() <= 0.9)
        continue;

      if (choose_mat < 0.8) {
        // diffuse, marbled with a random tint
        color tint(rnd() * rnd(), rnd() * rnd(), rnd() * rnd());
        auto marble = std::make_shared<noise_texture>(20.0, rng, tint);
        point3 center2 = center + vec3(0, rnd(0, 0.5), 0);
        world.add(std::make_shared<sphere>(
            center, center2, 0.2, std::make_shared<lambertian>(marble)));
      } else if (choose_mat < 0.95) {
        // metal
        color albedo(rnd(0.5, 1), rnd(0.5, 1), rnd(0.5, 1));
        double fuzz = rnd(0, 0.5);
        world.add(std::make_shared<sphere>(
            center, 0.2, std::make_shared<metal>(albedo, fuzz)));
      } else {
        // glass
        world.add(std::make_shared<sphere>(center, 0.2,
                                           std::make_shared<dielectric>(1.5)));
      }
    }
  }

  world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0,
                                     std::make_shared<dielectric>(1.5)));

  auto big_marble =
      std::make_shared<noise_texture>(4.0, rng, color(0.8, 0.5, 0.3));
  world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0,
                                     std::make_shared<lambertian>(big_marble)));

  world.add(std::make_shared<sphere>(
      point3(4, 1, 0), 1.0,
      std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0)));

  bvh_node bvh(world.get_hit_list());

  camera_settings cs;

  cs.aspect_ratio = 16.0 / 9.0;
  cs.image_width = 1200;
  cs.samples_per_pixel = 500;
  cs.max_depth = 100;

  cs.vfov = 20;
  cs.look_from = point3(13, 2, 3);
  cs.look_at = point3(0, 0, 0);
  cs.v_up = vec3(0, 1, 0);

  cs.defocus_angle = 0.6;
  cs.focus_dist = 10.0;
  camera cam(cs);
  cam.render(std::cout, bvh);
}

void checkered_spheres() {
  hittable_list world;

  auto checker = std::make_shared<checker_texture>(0.32, color(.2, .3, .1),
                                                   color(.9, .9, .9));

  world.add(std::make_shared<sphere>(point3(0, -10, 0), 10,
                                     std::make_shared<lambertian>(checker)));
  world.add(std::make_shared<sphere>(point3(0, 10, 0), 10,
                                     std::make_shared<lambertian>(checker)));

  camera_settings cs;

  cs.aspect_ratio = 16.0 / 9.0;
  cs.image_width = 400;
  cs.samples_per_pixel = 100;
  cs.max_depth = 50;

  cs.vfov = 20;
  cs.look_from = point3(13, 2, 3);
  cs.look_at = point3(0, 0, 0);
  cs.v_up = vec3(0, 1, 0);

  cs.defocus_angle = 0;

  camera cam(cs);

  cam.render(std::cout, world);
}

void earth() {
  auto earth_texture = std::make_shared<image_texture>("earthmap.jpg");
  auto earth_surface = std::make_shared<lambertian>(earth_texture);
  auto globe = std::make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

  camera_settings cs;

  cs.aspect_ratio = 16.0 / 9.0;
  cs.image_width = 400;
  cs.samples_per_pixel = 100;
  cs.max_depth = 50;

  cs.vfov = 20;
  cs.look_from = point3(0, 0, 12);
  cs.look_at = point3(0, 0, 0);
  cs.v_up = vec3(0, 1, 0);

  cs.defocus_angle = 0;

  camera cam(cs);
  cam.render(std::cout, hittable_list(globe));
}

void perlin_spheres() {
  hittable_list world;

  std::mt19937 rng(42);
  auto pertext = std::make_shared<noise_texture>(4, rng);

  world.add(std::make_shared<sphere>(point3(0, -1000, 0), 1000,
                                     std::make_shared<lambertian>(pertext)));
  world.add(std::make_shared<sphere>(point3(0, 2, 0), 2,
                                     std::make_shared<lambertian>(pertext)));

  camera_settings cs;

  cs.aspect_ratio = 16.0 / 9.0;
  cs.image_width = 400;
  cs.samples_per_pixel = 100;
  cs.max_depth = 50;

  cs.vfov = 20;
  cs.look_from = point3(13, 2, 3);
  cs.look_at = point3(0, 0, 0);
  cs.v_up = vec3(0, 1, 0);

  cs.defocus_angle = 0;
  camera cam(cs);
  cam.render(std::cout, world);
}