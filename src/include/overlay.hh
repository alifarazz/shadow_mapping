#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <array>

class Overlay {
  using v2 = glm::vec2;
  using Quad = std::array<v2, 4>;

public:
  GLuint VAO;

  Overlay(float);
  Overlay(Quad, float);
  Overlay(Overlay &&) = default;
  Overlay(const Overlay &) = default;
  Overlay &operator=(Overlay &&) = default;
  Overlay &operator=(const Overlay &) = default;
  ~Overlay();

  auto destory() -> void;

  auto draw() -> void;

private:
  Quad overlay_quad_vertices;
  Quad overlay_quad_uvs;
  GLuint VBO_uv, VBO_pos;

  auto setup(float) -> void;

  static constexpr Quad QUAD_UVs{v2{1.0f, 0.0f}, v2{1.0f, 1.0f}, v2{0.0f, 0.0f},
                                 v2{0.0f, 1.0f}};
  static constexpr Quad QUAD_VERTICES{
      v2{-0.5f, -1.0f},
      v2{-0.5f, -0.5f},
      v2{-1.0f, -1.0f},
      v2{-1.0f, -0.5f},
  };
};

Overlay::Overlay(float aspect_ratio) : Overlay{QUAD_VERTICES, aspect_ratio} {}

Overlay::Overlay(Quad overlay_quad_vertices, float aspect_ratio)
    : overlay_quad_vertices{overlay_quad_vertices}, overlay_quad_uvs{QUAD_UVs} {
  setup(aspect_ratio);
}

Overlay::~Overlay() {}

auto Overlay::setup(float aspect_ratio) -> void {
  for (auto &pos : overlay_quad_vertices)
    if (pos.y != -1.0f)
      pos.y /= aspect_ratio;

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO_uv);
  glGenBuffers(1, &VBO_pos);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(v2) * overlay_quad_vertices.size(),
               overlay_quad_vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_uv);
  glBufferData(GL_ARRAY_BUFFER, sizeof(v2) * overlay_quad_uvs.size(),
               overlay_quad_uvs.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

auto Overlay::destory() -> void {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO_pos);
  glDeleteBuffers(1, &VBO_uv);
}
auto Overlay::draw() -> void {
  glDrawArrays(GL_TRIANGLE_STRIP, 0, overlay_quad_vertices.size());
}
