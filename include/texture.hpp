#pragma once
#include "rtw_stb_image.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

class texture {
public:
  virtual ~texture() = default;

  virtual color value(double u, double v, const point3 &p) const = 0;
};

class solid_color : public texture {
public:
  explicit solid_color(const color &c) : albedo(c) {}
  solid_color(double r, double g, double b) : albedo(r, g, b) {}

  color value(double, double, const point3 &) const override { return albedo; }

private:
  color albedo;
};

class checker_texture : public texture {
public:
  checker_texture(double scale, std::shared_ptr<texture> even,
                  std::shared_ptr<texture> odd)
      : inv_scale(1 / scale), even(std::move(even)), odd(std::move(odd)) {}
  checker_texture(double scale, const color &even, const color &odd)
      : inv_scale(1 / scale), even(std::make_shared<solid_color>(even)),
        odd(std::make_shared<solid_color>(odd)) {}

  color value(double u, double v, const point3 &p) const override {
    int sum = 0;
    for (int i = 0; i < 3; i++) {
      sum += static_cast<int>(std::floor(p[i] * inv_scale));
    }
    return sum % 2 == 0 ? even->value(u, v, p) : odd->value(u, v, p);
  }

private:
  double inv_scale;
  std::shared_ptr<texture> even;
  std::shared_ptr<texture> odd;
};

class image_texture : public texture {
public:
  explicit image_texture(const char *filename) : image(filename) {}

  color value(double u, double v, const point3 &) const override {
    if (image.height() == 0) {
      return color(0, 1, 1);
    }
    u = std::clamp<double>(u, 0, 1);
    v = 1 - std::clamp<double>(v, 0, 1);
    int i = static_cast<int>(u * image.width());
    int j = static_cast<int>(v * image.height());
    constexpr double scale = 1.0 / 255;

    const unsigned char *pixel = image.pixel_data(i, j);
    return color(pixel[0] * scale, pixel[1] * scale, pixel[2] * scale);
  }

private:
  rtw_image image;
};
