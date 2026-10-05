#include "scenes.hpp"
#include "camera.hpp"
#include "hittable.hpp"
#include "vec3.hpp"
#include <cstdint>
#include <memory>

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

void quads() {
  hittable_list world;

  auto left_red = std::make_shared<lambertian>(color(1.0, 0.2, 0.2));
  auto back_green = std::make_shared<lambertian>(color(0.2, 1.0, 0.2));
  auto right_blue = std::make_shared<lambertian>(color(0.2, 0.2, 1.0));
  auto upper_orange = std::make_shared<lambertian>(color(1.0, 0.5, 0.0));
  auto lower_teal = std::make_shared<lambertian>(color(0.2, 0.8, 0.8));

  world.add(std::make_shared<planar_shape>(point3(-3, -2, 5), vec3(0, 0, -4),
                                           vec3(0, 4, 0), left_red));
  world.add(std::make_shared<planar_shape>(point3(-2, -2, 0), vec3(4, 0, 0),
                                           vec3(0, 4, 0), back_green));
  world.add(std::make_shared<planar_shape>(point3(3, -2, 1), vec3(0, 0, 4),
                                           vec3(0, 4, 0), right_blue));
  world.add(std::make_shared<planar_shape>(point3(-2, 3, 1), vec3(4, 0, 0),
                                           vec3(0, 0, 4), upper_orange));
  world.add(std::make_shared<planar_shape>(point3(-2, -3, 5), vec3(4, 0, 0),
                                           vec3(0, 0, -4), lower_teal));

  camera_settings cs;

  cs.aspect_ratio = 1.0;
  cs.image_width = 400;
  cs.samples_per_pixel = 100;
  cs.max_depth = 50;

  cs.vfov = 80;
  cs.look_from = point3(0, 0, 9);
  cs.look_at = point3(0, 0, 0);
  cs.v_up = vec3(0, 1, 0);

  cs.defocus_angle = 0;

  camera cam(cs);
  cam.render(std::cout, world);
}

void planar_shapes() {
  hittable_list world;

  auto red = std::make_shared<lambertian>(color(1.0, 0.2, 0.2));
  auto green = std::make_shared<lambertian>(color(0.2, 1.0, 0.2));
  auto blue = std::make_shared<lambertian>(color(0.2, 0.2, 1.0));

  vec3 u(2, 0, 0);
  vec3 v(0, 2, 0);

  world.add(std::make_shared<planar_shape>(point3(-4, -1, 0), u, v, red));
  world.add(std::make_shared<planar_shape>(point3(-1, -1, 0), u, v, green,
                                           planar_shape::kind::triangle));
  world.add(std::make_shared<planar_shape>(point3(2, -1, 0), u, v, blue,
                                           planar_shape::kind::ellipse));

  camera_settings cs;

  cs.aspect_ratio = 16.0 / 9.0;
  cs.image_width = 400;
  cs.samples_per_pixel = 100;
  cs.max_depth = 50;

  cs.vfov = 40;
  cs.look_from = point3(0, 0, 9);
  cs.look_at = point3(0, 0, 0);
  cs.v_up = vec3(0, 1, 0);

  cs.defocus_angle = 0;
  camera cam(cs);
  cam.render(std::cout, world);
}

void simple_light() {
  hittable_list world;

  std::mt19937 rng(42);
  auto pertext = std::make_shared<noise_texture>(4.0, rng);

  world.add(std::make_shared<sphere>(point3(0, -1000, 0), 1000,
                                     std::make_shared<lambertian>(pertext)));
  world.add(std::make_shared<sphere>(point3(0, 2, 0), 2,
                                     std::make_shared<lambertian>(pertext)));

  auto difflight = std::make_shared<diffuse_light>(color(4, 4, 4));
  world.add(std::make_shared<sphere>(point3(0, 7, 0), 2, difflight));
  world.add(std::make_shared<planar_shape>(point3(3, 1, -2), vec3(2, 0, 0),
                                           vec3(0, 2, 0), difflight));

  camera_settings settings;
  settings.image_width = 400;
  settings.aspect_ratio = 16.0 / 9.0;
  settings.vfov = 20;
  settings.look_from = point3(26, 3, 6);
  settings.look_at = point3(0, 2, 0);
  settings.v_up = vec3(0, 1, 0);
  settings.background_color = color(0, 0, 0);
  settings.samples_per_pixel = 100;
  settings.max_depth = 50;
  settings.defocus_angle = 0;

  camera cam(settings);
  cam.render(std::cout, world);
}

void cornell_box() {
  hittable_list world;

  auto red = std::make_shared<lambertian>(color(.65, .05, .05));
  auto white = std::make_shared<lambertian>(color(.73, .73, .73));
  auto green = std::make_shared<lambertian>(color(.12, .45, .15));
  auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

  auto wall = [&world](const point3 &q, const vec3 &u, const vec3 &v,
                       std::shared_ptr<material> mat) {
    world.add(std::make_shared<planar_shape>(q, u, v, mat));
  };

  wall(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green);
  wall(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red);
  wall(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), light);
  wall(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white);
  wall(point3(555, 555, 555), vec3(-555, 0, 0), vec3(0, 0, -555), white);
  wall(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white);

  world.add(box(point3(130, 0, 65), point3(295, 165, 230), white));
  world.add(box(point3(265, 0, 295), point3(430, 330, 460), white));

  camera_settings settings;
  settings.image_width = 600;
  settings.aspect_ratio = 1.0;
  settings.vfov = 40;
  settings.look_from = point3(278, 278, -800);
  settings.look_at = point3(278, 278, 0);
  settings.v_up = vec3(0, 1, 0);
  settings.background_color = color(0, 0, 0);
  settings.samples_per_pixel = 200;
  settings.max_depth = 50;

  camera cam(settings);
  cam.render(std::cout, world);
}

void standard_cornell() {
  hittable_list world;

  auto red = std::make_shared<lambertian>(color(.65, .05, .05));
  auto white = std::make_shared<lambertian>(color(.73, .73, .73));
  auto green = std::make_shared<lambertian>(color(.12, .45, .15));
  auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

  auto wall = [&world](const point3 &q, const vec3 &u, const vec3 &v,
                       std::shared_ptr<material> mat) {
    world.add(std::make_shared<planar_shape>(q, u, v, mat));
  };

  wall(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green);
  wall(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red);
  wall(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), light);
  wall(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white);
  wall(point3(555, 555, 555), vec3(-555, 0, 0), vec3(0, 0, -555), white);
  wall(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white);

  auto box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
  auto box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
  auto rot1 = std::make_shared<rotate>(box1, 15);
  auto trans1 = std::make_shared<translate>(rot1, point3(265, 0, 295));
  auto rot2 = std::make_shared<rotate>(box2, -18);
  auto trans2 = std::make_shared<translate>(rot2, vec3(130, 0, 65));

  world.add(trans1);
  world.add(trans2);

  camera_settings settings;
  settings.image_width = 600;
  settings.aspect_ratio = 1.0;
  settings.vfov = 40;
  settings.look_from = point3(278, 278, -800);
  settings.look_at = point3(278, 278, 0);
  settings.v_up = vec3(0, 1, 0);
  settings.background_color = color(0, 0, 0);
  settings.samples_per_pixel = 1000;
  settings.max_depth = 50;

  camera cam(settings);
  cam.render(std::cout, world);
}
