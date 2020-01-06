#pragma once

#include <glad/glad.h>

#include "utils.hh"

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class Texture {
public:
  enum Type {
    Diffuse,
    Normal,
    Specular,
    None,
  };

  auto destory() -> void { glDeleteTextures(1, &id); }

  GLuint id;
  Type type;

  static auto typeName(const Type t) -> std::string {
    switch (t) {
    case Type::Diffuse:
      return "Diffuse";
    case Type::Normal:
      return "Normal";
    case Type::Specular:
      return "Specular";
    default:
      break;
    }
    return "None";
  }
};
