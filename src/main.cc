#include <GLFW/glfw3.h>
#include <glad/glad.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

#include <shader.hh>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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


  // Adjust viewport upon window resize
  glfwSetFramebufferSizeCallback(
      window.get(), [](auto, int w, int h) { glViewport(0, 0, w, h); });
  // Grab cursor
  glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);


  /* SHADERS */
  Shader shader{};
  shader.attach(fs::current_path() / "shader/empty.vert", GL_VERTEX_SHADER)
      .attach(fs::current_path() / "shader/mono.frag", GL_FRAGMENT_SHADER)
      .link();

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

  glUseProgram(shader.id());
  GLuint mvp_uniform_location = shader.getUniform("mvp");


  auto prespective_projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
  auto cam_pos{glm::vec3(0, 0,-1)};
  auto cam_up{glm::vec3(0, 1, 0)};
  glm::dvec2 cursor_pos;
  float keyboard_speed = 0.01;

  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_FRONT);
  while (!glfwWindowShouldClose(window.get())) {
    if (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window.get(), true);

    auto cam_fwd {glm::vec3(0, 0, 1)};
    auto cam_right{glm::normalize(glm::cross(cam_up, cam_fwd))};

    glfwGetCursorPos(window.get(), &(cursor_pos.x), &(cursor_pos.y));
    glm::vec3 cam_target =
            glm::rotate(glm::mat4(1.0f),
                    static_cast<float>(cursor_pos.x) * 0.2f * keyboard_speed,
                    -cam_up) *
        glm::rotate(glm::mat4(1.0f),
                    static_cast<float>(cursor_pos.y) * 0.2f * keyboard_speed,
                    cam_right) *
            glm::vec4{cam_fwd, 0.0f};

    auto cam_right_2{glm::normalize(glm::cross(cam_up, cam_target))};

    if (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS)
      cam_pos += cam_target * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS)
      cam_pos += cam_right_2 * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS)
      cam_pos -= cam_right_2 * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS)
      cam_pos -= cam_target * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_I) == GLFW_PRESS)
      cam_pos += cam_up * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_K) == GLFW_PRESS)
      cam_pos -= cam_up * keyboard_speed;

    // cam_up = glm::rotate(
    //     glm::mat4(1.0f), static_cast<float>(cursor_pos.x) * keyboard_speed,
    //     -cam_right) * glm::vec4{cam_up, 0.0f};
    // glm::mat4 mvp;
    // mvp[0] = glm::vec4{cam_right, -cam_pos[0]};
    // mvp[1] = glm::vec4{cam_up, -cam_pos[1]};
    // mvp[2] = glm::vec4{cam_fwd, -cam_pos[2]};
    // mvp[3] = glm::vec4{0, 0, 0, 1};
    auto view_projection{glm::lookAt(cam_pos, cam_pos + cam_target, cam_up)};
    auto mvp = prespective_projection * view_projection;

    glUniformMatrix4fv(mvp_uniform_location, 1, GL_FALSE, glm::value_ptr(mvp));

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader.id());
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwPollEvents();
    glfwSwapBuffers(window.get());
  }

  /* CLEAN-UP */
  glDeleteProgram(shader.id());
  glfwTerminate();
  return 0;
}
