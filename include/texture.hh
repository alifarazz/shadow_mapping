#pragma once

#include <glad/glad.h>

#include <string>

class Texture {
public:
  enum Type {
    Diffuse,
    Normal,
    Specular,
    None,
  };

  ~Texture() { glDeleteTextures(1, &id); }

  GLuint id;
  Type type;

  static auto typeName(Type t) -> std::string {
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
