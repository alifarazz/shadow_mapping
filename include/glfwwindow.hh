#pragma once

#include <GLFW/glfw3.h>
#include <memory>

struct DestoryGLFWWindowStruct{
  auto operator()(GLFWwindow *ptr) -> void {glfwDestroyWindow(ptr);}
};

using GLFWwindowUniquePtr = std::unique_ptr<GLFWwindow, DestoryGLFWWindowStruct>;
