#pragma once

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <tuple>
namespace fs = std::filesystem;

class Utils {
protected:
  static auto free_image(stbi_uc *x) -> void { stbi_image_free(x); }

public:
  static auto loadTextureFromFile(const fs::path &path,
                                  bool gammaCorrect = true) -> GLuint {

    GLuint textureID;
    glGenTextures(1, &textureID);
    auto [data, w, h, nrComponents] = loadImageFromFile(path); // fix data_tmp
    // std::unique_ptr<stbi_uc *, decltype(free_image)> data{data_tmp, &free_image};

    if (data) {
      GLenum format{};
      if (nrComponents == 1)
        format = GL_RED;
      else if (nrComponents == 3)
        format = GL_RGB;
      else if (nrComponents == 4)
        format = GL_RGBA;

      glBindTexture(GL_TEXTURE_2D, textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE,
                   data);
                   // data.get());
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
      std::cerr << "TEXTURE::LOAD::FAILED TO LOAD AT " << path << std::endl;
      glDeleteTextures(1, &textureID); // don't leak texture
    }

    free_image(data);
    return textureID;
  }

  static auto loadImageFromFile(const fs::path &path)
    -> std::tuple<stbi_uc *,
                  int,
                  int,
                  int> {
    int x, y, comp;
    stbi_uc * data = stbi_load(path.c_str(), &x, &y, &comp, 0);
// std::unique_ptr<stbi_uc *, decltype(free_image)> &&
// std::unique_ptr<stbi_uc *, decltype(free_image)>(u, free_image).get()
    return std::make_tuple(data, x, y, comp);
  }
};
