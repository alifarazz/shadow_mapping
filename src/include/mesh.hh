#pragma once

// #include "texture.hh"

// #include <glad/glad.h>

#include "texture.hh"
#include "vertex.hh"

#include "shader.hh"

#include <glad/glad.h>

#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
namespace fs = std::filesystem;

class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<uint> indices;
std::vector<Texture> textures;

  Mesh(const std::vector<Vertex> &vertices, const std::vector<uint> &indices,
       const std::vector<Texture> &textures);
  Mesh(std::vector<Vertex> &vertices, std::vector<uint> &idices,
       std::vector<Texture> &textures);

  // COPY
  Mesh(const Mesh& other) = delete;
  Mesh& operator=(const Mesh& other) = delete;

  // MOVE
  Mesh(Mesh&& other) noexcept { *this = std::move(other); }
  Mesh& operator=(Mesh&& other);

  auto draw(GLenum drawMode) -> void;
  auto bindVAO() const -> void;
  auto bindTextures(const Shader &shader, uint offsetTexture) -> void;
  auto bindDraw(const Shader& shader, uint offsetTexture, GLenum drawMode) -> void;

  auto destory() -> void;

private:
  uint VAO, VBO, EBO;
  auto setupMesh() -> void;
};

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<uint> &indices,
           const std::vector<Texture> &textures)
    : vertices{vertices}, indices{indices}, textures{textures} {
  setupMesh();
}
Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<uint> &indices,
           std::vector<Texture> &textures)
    : vertices{vertices}, indices{indices}, textures{textures} {
  setupMesh();
}

Mesh& Mesh::operator=(Mesh&& other){
  if (this != &other) {
    vertices = std::move(other.vertices);
    indices = std::move(other.indices);
    textures = std::move(other.textures);
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
  }
  return *this;
}

auto Mesh::setupMesh() -> void {
  constexpr auto sizeofVertices{sizeof(decltype(vertices)::value_type)};
  constexpr auto sizeofIndecies{sizeof(decltype(indices)::value_type)};

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeofVertices,
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeofIndecies,
               indices.data(), GL_STATIC_DRAW);

  // 0: vertex positions
  // 1: vertex normal
  // 2: vertex texture coords
  for (uint i = 0; i < 3; i++)
    glEnableVertexAttribArray(i);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, position)));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, normal)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, texCoords)));

  // glBindVertexArray(0);
}

auto Mesh::bindVAO() const -> void {
  glBindVertexArray(VAO);
}

auto Mesh::bindTextures(const Shader &shader, uint offsetTexture = 0) -> void{
  using namespace std::string_literals;

  uint diffuseN{1}, specularN{1}, normalN{1};
  for (uint i = offsetTexture; i < textures.size() + offsetTexture; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    uint idx = i - offsetTexture;

    std::string name;
    switch (textures[idx].type) {
    case Texture::Type::Diffuse:
      name = "texture_diffuse" + std::to_string(diffuseN++);
      // std::clog <<name;
      break;
    case Texture::Type::Specular:
      // static bool flg{false};
      // if (!flg) {
      //   std::clog << "spec unit: " << i << std::endl;
      //   flg = true;
      // }
      name = "texture_specular" + std::to_string(specularN++);
      // std::clog <<name;
      break;
    case Texture::Type::Normal:
      name = "texture_normal" + std::to_string(normalN++);
      // std::clog <<name;
      break;
      // case
    default:
      name = "INVALID";
      std::clog << "INVALIND NAME FOR TEXTURE: " << shader.id() << ':' << idx
                << ", TYPE: " << textures[idx].type << std::endl;
    }
    auto samplerLocation{glGetUniformLocation(shader.id(), ("material."s + name).c_str())};
    // std::clog << "--> " << "material." + name << ": " << samplerLocation << std::endl;
    glUniform1i(samplerLocation, i);
    glBindTexture(GL_TEXTURE_2D, textures[idx].id);
  }
  glActiveTexture(GL_TEXTURE0);
}
auto Mesh::draw(GLenum drawMode=GL_TRIANGLES) -> void {
  glDrawElements(drawMode, indices.size(), GL_UNSIGNED_INT, 0);
}

auto Mesh::bindDraw(const Shader& shader, uint offsetTexture = 0, GLenum drawMode=GL_TRIANGLES) -> void {
  bindVAO();
  bindTextures(shader, offsetTexture);
  draw(drawMode);
}

auto Mesh::destory() -> void {
  for (auto &t : textures)
    t.destory();
  for (auto &v : vertices)
    v.destory();
  glDeleteBuffers(1, &VAO);
  glDeleteBuffers(1, &EBO);
  glDeleteBuffers(1, &VBO);
}