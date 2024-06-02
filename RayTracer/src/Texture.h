#pragma once
#include "rtw_image.h"
#include "glm/glm.hpp"


class Texture {
public:
    std::string textureName;

    //Default constructor, no texture data
    Texture() : image() { textureName = "Empty Texture"; };
    //Actual texture data
    Texture(const char* filename) : image(filename) { textureName = std::string(filename); };

    glm::vec3 value(float u, float v) const {
        // If we have no texture data, then return solid white so material albedo will scale it.
        if (image.height() <= 0) return glm::vec3{ 1, 1, 1 };

        auto i = static_cast<int>(u * image.width());
        auto j = static_cast<int>(v * image.height());
        auto pixel = image.pixel_data(i, j);

        auto color_scale = 1.0 / 255.0;
        return glm::vec3{ color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2] };
    }

private:
    rtw_image image;
};