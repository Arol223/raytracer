#include "scenes.hpp"

int main() {
  switch (9) {
  case 1:
    bouncing_spheres();
    break;
  case 2:
    checkered_spheres();
    break;
  case 3:
    earth();
    break;
  case 4:
    perlin_spheres();
    break;
  case 5:
    quads();
    break;
  case 6:
    planar_shapes();
    break;
  case 7:
    simple_light();
    break;
  case 8:
    cornell_box();
    break;
  case 9:
    standard_cornell();
    break;
  }

  return 0;
}
