#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glfwwindow.hh"


class FPSCamera {
public:
  glm::vec3 cameraPos;
  glm::vec3 cameraFwd;
  glm::vec3 cameraUp;
  glm::vec3 cameraRight;
  glm::vec3 worldUp;
  glm::vec3 worldFwd; // used for the mouse
  glm::vec3 worldRight; // used for mouse

  // float yaw, pitch;

  glm::mat4 model_transform;
  glm::mat4 view_transform;
  glm::mat4 prespective_transform;

  float aspect_ratio;           // in radians
  float fov;

  float movementSpeed;
  float mouseSensitivity;


  enum {UP, DOWN, LEFT, RIGHT} CameraDirection;

  FPSCamera(float aspect_ratio,
            // float yaw = YAW,
            // float pitch = PITCH,
            float fov = FOV):
    cameraPos       {CAMERAPOS},
    worldUp         {CAMERAUP},
    worldFwd        {CAMERAFWD},
    worldRight      {glm::cross(worldUp, worldFwd)},
    // yaw             {yaw},
    // pitch           {pitch},
    aspect_ratio    {aspect_ratio},
    fov             {fov},
    movementSpeed   {KEYBOARD_SPEED},
    mouseSensitivity{CURSOR_SPEED}
  {updateDependentStuff();}

  FPSCamera(glm::vec3 cameraPos,
            glm::vec3 cameraFwd,
            glm::vec3 worldUp,
            float aspect_ratio,
            // float yaw = YAW,
            // float pitch = PITCH,
            float fov = FOV):
    cameraPos       {cameraPos},
    worldUp         {worldUp},
    worldFwd        {cameraFwd},
    worldRight      {glm::cross(worldUp, cameraFwd)},
    // yaw             {yaw},
    // pitch           {pitch},
    aspect_ratio    {aspect_ratio},
    fov             {fov},
    movementSpeed   {KEYBOARD_SPEED},
    mouseSensitivity{CURSOR_SPEED}
  {updateDependentStuff();}

  // auto pollKeyboard() -> void;
  // auto pollMouse() -> void;
  // auto pollInput() -> void;

  auto updateModelMatrix() -> void;
  auto updateViewMatrix() -> void;
  auto updatePrespectiveMatrix() -> void;
  auto updateCameraRight() -> void;
  auto updateDependentStuff() -> void;
  auto updateWorldCameraRight() -> void;
  auto renderloopUpdateMVP(GLFWwindowUniquePtr &window, float deltaTime) -> void;

  auto updateMVP(GLFWwindowUniquePtr &window, float deltaTime) -> void;
  auto uploadMVP(GLuint, GLuint, GLuint) -> void;

  auto processKeyboard(GLFWwindowUniquePtr &window, float deltaTime) -> void;
  auto processCurosr(GLFWwindowUniquePtr &window, float deltaTime) -> void;

  // Default camera values
  static constexpr auto YAW            = -90.0f;
  static constexpr auto PITCH          = 0.0f;
  static constexpr auto KEYBOARD_SPEED = 2.5f;
  static constexpr auto CURSOR_SPEED   = 0.5f;
  static constexpr auto SENSITIVITY    = 0.1f;
  static constexpr auto ASPECT_RATIO   = 45.0f;
  static constexpr auto CAMERAPOS      = glm::vec3{0.0f, 0.0f, 0.0f};
  static constexpr auto CAMERAFWD      = glm::vec3{0.0f, 0.0f, -1.0f};
  static constexpr auto CAMERAUP       = glm::vec3{0.0f, 1.0f, 0.0f};
  static constexpr auto FOV            = glm::radians(45.0f);

private:
  static auto uploadMatrix(GLuint uniform_location, glm::mat4 mat) -> void {
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(mat));
  }
};


// auto FPSCamera::pollKeyboard() ->void {}
auto FPSCamera::updateModelMatrix() -> void {
  model_transform = glm::mat4(1.0f);
}

auto FPSCamera::updateViewMatrix() -> void {
    view_transform = glm::lookAt(cameraPos, cameraPos + cameraFwd, worldUp);
}

auto FPSCamera::updatePrespectiveMatrix() -> void {
   prespective_transform = glm::perspective(fov,
                                            aspect_ratio,
                                            0.01f,
                                            100.0f);
}

auto FPSCamera::updateCameraRight() -> void {
  cameraRight = glm::normalize(glm::cross(worldUp, cameraFwd));
}

auto FPSCamera::updateWorldCameraRight() -> void {
  worldRight = glm::cross(worldUp, worldFwd);
}

// The order of the function call DO matter!
auto FPSCamera::updateMVP(GLFWwindowUniquePtr &window, float deltaTime) -> void{
  updateWorldCameraRight();  // not needed for each call, only when default{Fwd,Right} are updated
  processCurosr(window, deltaTime);  // updates cameraFwd
  updateCameraRight();  // updates cameraRight
  processKeyboard(window, deltaTime); // offset cameraPos
  updatePrespectiveMatrix();  // not needed for each call
  updateViewMatrix();
  updateModelMatrix();
}

auto FPSCamera::updateDependentStuff() -> void {
  updateWorldCameraRight(); // update worldRight
  cameraFwd = glm::rotate(glm::mat4(1.0f), 0.0f, -worldUp)
            * glm::rotate(glm::mat4(1.0f), 0.0f, worldRight)
            * glm::vec4{worldFwd, 0.0f};
  updateCameraRight();
  updateViewMatrix();
  updateModelMatrix();
  updatePrespectiveMatrix();
}

auto FPSCamera::renderloopUpdateMVP(GLFWwindowUniquePtr &window, float deltaTime) -> void {
  processCurosr(window, deltaTime);  // updates cameraFwd
  updateCameraRight();  // updates cameraRight
  processKeyboard(window, deltaTime); // offset cameraPos
  updateViewMatrix();
  // updateModelMatrix();
}

auto FPSCamera::uploadMVP(GLuint model_uniform_location,
                          GLuint view_uniform_location,
                          GLuint prespective_uniform_location) -> void {
    uploadMatrix(model_uniform_location, model_transform);
    uploadMatrix(view_uniform_location, view_transform);
    uploadMatrix(prespective_uniform_location, prespective_transform);
}

// TODO: the sensitvity felt for each axis will depend on aspect ratio. FIX PLS
auto FPSCamera::processCurosr(GLFWwindowUniquePtr &window, float deltaTime) -> void {
  glm::dvec2 cursor_pos;
  float v = mouseSensitivity * 0.01; //* deltaTime;

  glfwGetCursorPos(window.get(), &(cursor_pos.x), &(cursor_pos.y));

  auto x_angle = static_cast<float>(cursor_pos.x) * v,
       y_angle = static_cast<float>(cursor_pos.y) * v;
  cameraFwd = glm::rotate(glm::mat4(1.0f), x_angle, -worldUp)
            * glm::rotate(glm::mat4(1.0f), y_angle, worldRight)
            * glm::vec4{worldFwd, 0.0f};
}

// PLS call this after calling processCursor (so that the cameraFwd is updated).
auto FPSCamera::processKeyboard(GLFWwindowUniquePtr &window, float deltaTime) -> void {
  float v = movementSpeed * deltaTime;

  if (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraFwd * v;
  if (glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS)
    cameraPos += cameraRight * v;
  if (glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS)
    cameraPos -= cameraRight * v;
  if (glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraFwd * v;
  if (glfwGetKey(window.get(), GLFW_KEY_SPACE) == GLFW_PRESS)
    cameraPos += worldUp * v;
  if (glfwGetKey(window.get(), GLFW_KEY_C) == GLFW_PRESS)
    cameraPos -= worldUp * v;
}
