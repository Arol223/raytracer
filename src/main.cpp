#include "scenes.hpp"

int main() {
  switch (4) {
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
  }
  return 0;
}
