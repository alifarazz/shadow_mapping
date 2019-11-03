#pragma once
#include <unordered_map>
// #include <optional>

#include "texture.hh"


class Scene{
public:
  std::unordered_map<std::string, Texture> loadedTextures;



};
