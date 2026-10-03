#include "scenes.hpp"
#include "camera.hpp"
#include <cstdint>

void bouncing_spheres(std::uint32_t seed) {
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
  // aspect ratio 16:9
  camera_settings cs = camera_settings();
  cs.aspect_ratio = 16.0 / 9.0;
  cs.image_width = 1200;
  cs.look_from = point3(13, 2, 3);
  cs.look_at = point3(0, 0, 0);
  cs.v_up = vec3(0, 1, 0);
  cs.vfov = 20;
  cs.max_depth = 100;
  cs.samples_per_pixel = 500;
  cs.focus_dist = 10;
  cs.defocus_angle = 0.6;
  camera c = camera(cs);
  bvh_node BVH = bvh_node(world.get_hit_list());
  c.render(std::cout, BVH);
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