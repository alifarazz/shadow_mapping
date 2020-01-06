#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils.hh"


#include <functional>


class FPSCamera {
public:
  glm::vec3 cameraPos;
  glm::vec3 cameraFwd;
  glm::vec3 cameraUp;
  glm::vec3 cameraRight;
  glm::vec3 worldUp;

  float yaw, pitch;

  glm::mat4 view_matrix;
  glm::mat4 prespective_matrix;
  float aspect_ratio;           // in radians
  float fov;

  float movementSpeed;
  float mouseSensitivity;

  FPSCamera(float aspect_ratio,
      float yaw = YAW,
      float pitch = PITCH,
      float fov = FOV)
      : FPSCamera(CAMERAPOS,
          CAMERAUP,
          aspect_ratio,
          yaw,
          pitch,
          fov)
  {
  }

  FPSCamera(glm::vec3 cameraPos,
            glm::vec3 worldUp,
            float aspect_ratio,
            float yaw = YAW,
            float pitch = PITCH,
            float fov = FOV):
    cameraPos       {cameraPos},
    worldUp         {worldUp},
    yaw             {yaw},
    pitch           {pitch},
    aspect_ratio    {aspect_ratio},
    fov             {fov},
    movementSpeed   {KEYBOARD_SPEED},
    mouseSensitivity{CURSOR_SPEED}
  {
    updateVectors();
    updateViewMatrix();
    updatePrespectiveMatrix();
  }

  auto updateViewMatrix() -> void;
  auto updatePrespectiveMatrix() -> void;

  auto updateCameraFwd() -> void;
  auto updateCameraUp() -> void;
  auto updateCameraRight() -> void;

  auto updateVectors() -> void;

  auto renderloopUpdateView(Utils::GLFWwindowUniquePtr &window, float deltaTime) -> void;

  auto processKeyboard(Utils::GLFWwindowUniquePtr &window, float deltaTime) -> void;
  auto processCurosr(float dx, float dy) -> void;
  auto mouseCallback(GLFWwindow *, double x, double y) -> void;
private:
  // Default camera values
  static constexpr auto YAW            = -90.0f;
  static constexpr auto PITCH          = 0.0f;
  static constexpr auto KEYBOARD_SPEED = 2.5f;
  static constexpr auto CURSOR_SPEED   = 0.5f;
  static constexpr auto SENSITIVITY    = 0.1f;
  static constexpr auto ASPECT_RATIO   = 45.0f;
  static constexpr auto CAMERAPOS      = glm::vec3{0.0f, 0.0f, 0.0f};
  static constexpr auto CAMERAUP       = glm::vec3{0.0f, 1.0f, 0.0f};
  static constexpr auto FOV            = glm::radians(45.0f);
};

auto FPSCamera::updateViewMatrix() -> void {
    view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFwd, cameraUp);
}

auto FPSCamera::updatePrespectiveMatrix() -> void {
  prespective_matrix = glm::perspective(fov, aspect_ratio, 0.0001f, 1000.0f);
}


auto FPSCamera::updateCameraFwd() -> void {
  auto rpitch{glm::radians(pitch)};
  auto ryaw  {glm::radians(yaw)};

  auto sy{std::sin(ryaw)};
  auto cy{std::cos(ryaw)};
  auto sp{std::sin(rpitch)};
  auto cp{std::cos(rpitch)};

  cameraFwd.x = cy * cp;
  cameraFwd.y = sp;
  cameraFwd.z = sy * cp;
}

auto FPSCamera::updateCameraRight() -> void {
  cameraRight = glm::normalize(glm::cross(cameraFwd, worldUp));
}

auto FPSCamera::updateCameraUp() -> void {
  cameraUp = glm::normalize(glm::cross(cameraRight, cameraFwd));
}

auto FPSCamera::updateVectors() -> void {
  updateCameraFwd();
  updateCameraRight();
  updateCameraUp();
}

auto FPSCamera::renderloopUpdateView(Utils::GLFWwindowUniquePtr &window, float deltaTime) -> void {
  // The order of the function call DO matter!
  processKeyboard(window, deltaTime); // move camera -> update cameraPos
  updateViewMatrix();
}

auto FPSCamera::mouseCallback(GLFWwindow *, double x, double y) -> void {
    static float lastX, lastY;
    static bool first = true;
    if (__builtin_expect(first, 0)) { // unlikely to be true
      lastX = x;
      lastY = y;
      first = false;
    }
    processCurosr(x - lastX, -y + lastY);
    lastX = x;
    lastY = y;
}
// TODO: the sensitvity felt for each axis will depend on aspect ratio. FIX PLS
auto FPSCamera::processCurosr(float dx, float dy) -> void {
  float v = mouseSensitivity * 0.2f; //* deltaTime;
  dx *= v;
  dy *= v;

  yaw += dx;
  pitch += dy;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;
  updateVectors();   // update cameraUp, cameraRight and cameraFwd
}

// PLS call this after calling processCursor (so that the cameraFwd is updated).
auto FPSCamera::processKeyboard(Utils::GLFWwindowUniquePtr &window, float deltaTime) -> void {
  float v = movementSpeed * deltaTime;
  if (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraFwd * v;
  if (glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += cameraRight * v;
  if (glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= cameraRight * v;
  if (glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraFwd * v;
  if (glfwGetKey(window.get(), GLFW_KEY_SPACE) == GLFW_PRESS)
    cameraPos += worldUp * v;
  if (glfwGetKey(window.get(), GLFW_KEY_C) == GLFW_PRESS)
    cameraPos -= worldUp * v;
}
