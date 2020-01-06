#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glfwwindow.hh"


class Looker {
  glm::vec3 pos;
  glm::vec3 fwd;
  glm::vec3 up;
  glm::vec3 right;
  const glm::vec3 worldUp;

  float yaw, pitch;

  glm::mat4 model_transform;
  glm::mat4 view_transform;
  glm::mat4 prespective_transform;

  auto updateModelMatrix() -> void;
  auto updateViewMatrix() -> void;
  auto updatePrespectiveMatrix() -> void;

  auto updateFwd() -> void;
  auto updateUp() -> void;
  auto updateRight() -> void;

  auto updateVectors() -> void;

};
auto Looker::updateModelMatrix() -> void {
  model_transform = glm::mat4(1.0f);
}

auto Looker::updateViewMatrix() -> void {
    view_transform = glm::lookAt(pos, pos + fwd, up);
}

auto Looker::updateFwd() -> void {
  auto rpitch{glm::radians(pitch)};
  auto ryaw  {glm::radians(yaw)};

  auto sy{std::sin(ryaw)};
  auto cy{std::cos(ryaw)};
  auto sp{std::sin(rpitch)};
  auto cp{std::cos(rpitch)};

  fwd.x = cy * cp;
  fwd.y = sp;
  fwd.z = sy * cp;
}
