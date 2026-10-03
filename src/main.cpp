#include "scenes.hpp"

int main() {
  switch (3) {
  case 1:
    bouncing_spheres();
    break;
  case 2:
    checkered_spheres();
    break;
  case 3:
    earth();
    break;
  }
  return 0;
}
