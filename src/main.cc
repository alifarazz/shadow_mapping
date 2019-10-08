#include <GLFW/glfw3.h>
#include <glad/glad.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

#include <shader.hh>

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
namespace fs = std::filesystem;

static std::array<float, 9> vertices{
    -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f,
};

static std::array<float, 9> colors{
    1, 0, 0, 0, 1, 0, 0, 0, 1,
};

auto destory_glfw_window(GLFWwindow *ptr) -> void { glfwDestroyWindow(ptr); }

int main() {
  int windowHeight{800}, winWidth{600};

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  std::unique_ptr<GLFWwindow, decltype(&destory_glfw_window)> window{
      glfwCreateWindow(windowHeight, winWidth, "ogl", nullptr, nullptr),
      &destory_glfw_window};
  glfwMakeContextCurrent(window.get());
  if (!window.get())
    throw std::runtime_error{"glfw window create failed"};
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
    throw std::runtime_error{"GLAD init failed"};

  // VBO + VAO
  GLuint vboVertex, vboColor;
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vboVertex);
  glGenBuffers(1, &vboColor);

  glBindVertexArray(vao);  // bind vertex array (VAO)

  // vboVertex
  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);  // bind buffer
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]),
               vertices.data(), GL_STATIC_DRAW);  // specify vertices
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,
                        nullptr); // specify layout in shader
  glEnableVertexAttribArray(0);   // `pos` in shader/empty.vert

  // vboColor
  glBindBuffer(GL_ARRAY_BUFFER, vboColor);
  glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]),
               colors.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(1); // `color` in shader/empty.vert

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind buffer
  glBindVertexArray(0);             // unbind vertex array (VAO)

  /* SHADERS */
  Shader shader{};
  shader.attach(fs::current_path() / "shader/empty.vert", GL_VERTEX_SHADER)
      .attach(fs::current_path() / "shader/mono.frag", GL_FRAGMENT_SHADER)
      .link();

  // Adjust viewport upon window resize
  glfwSetFramebufferSizeCallback(
      window.get(), [](auto, int w, int h) { glViewport(0, 0, w, h); });

  /* RENDER */
  // set-up shader and scene
  glUseProgram(shader.id());
  glBindVertexArray(vao);

  // set background
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  while (!glfwWindowShouldClose(window.get())) {
    if (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window.get(), true);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwPollEvents();
    glfwSwapBuffers(window.get());
  }

  /* CLEAN-UP */
  glDeleteProgram(shader.id());
  glfwTerminate();
  return 0;
}
