#pragma once

#include <glad/glad.h>

#include "texture.hh"

#include <filesystem>
#include <unordered_map>
namespace fs = std::filesystem;

class TextureRepository {
public:
  auto insert(std::pair<std::string, GLuint> i) -> bool;
  auto get(const std::string &p) -> GLuint;
private:
  std::unordered_map<std::string, GLuint> LoadedTextures;

  static auto loadTexture(const fs::path&) -> GLuint;
};

auto TextureRepository::insert(std::pair<std::string, GLuint> i) -> bool {
  return LoadedTextures.insert(i).second;
}

// TODO: use std::optional reference
auto TextureRepository::get(const std::string &p)
    -> GLuint { // TODO
  auto search{LoadedTextures.find(p)};
  const bool found = search != std::end(LoadedTextures);
  GLuint tex;
  if (!found) {
    tex = loadTexture(fs::path(p));
    this->insert({p, tex});
  } else {
    tex = search->second;
  }
  return tex;
}

auto TextureRepository::loadTexture(const fs::path &path) -> GLuint {
  GLuint textureID;
  glGenTextures(1, &textureID);
  auto [data, w, h, nrComponents] =
      Utils::loadImageFromFile(path); // fix data_tmp
  // stbi_uc_UniquePtr data { std::move(data_tmp) };

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
    std::clog << "LOG::Model::Utils::\"Loading Texture Successful\": " << path
              << std::endl;
    stbi_image_free(data);
    return textureID;
  }
  std::cerr << "TEXTURE::LOAD::FAILED TO LOAD AT " << path << std::endl;
  glDeleteTextures(1, &textureID); // don't leak texture
  return -1;
}

static TextureRepository DefaultTexRepo;
