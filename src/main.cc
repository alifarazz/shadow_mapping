#include <GLFW/glfw3.h>
#include <glad/glad.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.hh"
#include "shader.hh"
// #include "shader.hh"

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
namespace fs = std::filesystem;

// static std::array<glm::vec3, 4> vertices
// {
//  glm::vec3(-1.0f, -1.0f, 0.0f),
//  glm::vec3(-0.0f, -1.0f, 1.0f),
//  glm::vec3(1.0f, -1.0f, 0.0f),
//  glm::vec3(0.0f, 0.0f, .33f),
// };

// static std::array<glm::i32vec3, 4> indices
// {
//  glm::i32vec3(0, 3, 1),
//  glm::i32vec3(1, 3, 2),
//  glm::i32vec3(2, 3, 0),
//  glm::i32vec3(0, 1, 2),
// };

// static std::array<float, 12> colors{
//     1, 0, 0,
//     0, 1, 0,
//     0, 0, 1,
//     1, 1, 0,
// };

auto destory_glfw_window(GLFWwindow *ptr) -> void { glfwDestroyWindow(ptr); }

int main()
{
  int windowHeight{1366}, winWidth{768};

  // std::setlocale(LC_ALL, "POSIX");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  std::unique_ptr<GLFWwindow, decltype(&destory_glfw_window)> window{
      glfwCreateWindow(windowHeight, winWidth, "", nullptr, nullptr),
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
  Shader shader{}, shaderNanoSuit{};
  // shader.attach(fs::current_path() / "shader/empty.vert", GL_VERTEX_SHADER)
  //     .attach(fs::current_path() / "shader/mono.frag", GL_FRAGMENT_SHADER)
  //     .link();
  shaderNanoSuit
      .attach(fs::current_path() / "shader/lambertian.vert", GL_VERTEX_SHADER)
      .attach(fs::current_path() / "shader/texture.frag", GL_FRAGMENT_SHADER)
      .link();

  // // VBO + VAO
  // GLuint vboVertex, vboColor;
  // GLuint vao;
  // GLuint ibo;
  // glGenVertexArrays(1, &vao);
  // glGenBuffers(1, &vboVertex);
  // glGenBuffers(1, &vboColor);
  // glGenBuffers(1, &ibo);

  // glBindVertexArray(vao);  // bind vertex array (VAO)

  // // bind ibo to save vbos
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER,
  //              indices.size() * sizeof(decltype(indices)::value_type),
  //              indices.data(), GL_STATIC_DRAW);

  // // vboVertex
  // glBindBuffer(GL_ARRAY_BUFFER, vboVertex); // bind buffer
  // glBufferData(GL_ARRAY_BUFFER,
  //              vertices.size() * sizeof(decltype(vertices)::value_type),
  //              vertices.data(), GL_STATIC_DRAW); // specify vertices
  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,
  //                       nullptr); // specify layout in shader
  // glEnableVertexAttribArray(0);   // `pos` in shader/empty.vert

  // // vboColor
  // glBindBuffer(GL_ARRAY_BUFFER, vboColor);
  // glBufferData(GL_ARRAY_BUFFER,
  //              colors.size() * sizeof(decltype(colors)::value_type),
  //              colors.data(), GL_STATIC_DRAW);
  // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  // glEnableVertexAttribArray(1); // `color` in shader/empty.vert

  // glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind buffer
  // glBindVertexArray(0);             // unbind vertex array (VAO)

  /* LOADING MODELS */
  // Model pyramid{fs::current_path() / fs::path("res/suzzane/Pyramid.obj")};
  // Model suzzane{fs::current_path() / fs::path("res/suzzane/suzzane.glb")};
  Model suzzane{fs::current_path() / fs::path("res/nanosuit/nanosuit.obj")};

  glUseProgram(shaderNanoSuit.id());
  GLuint model_to_view_uniform_location = shaderNanoSuit.getUniform("modelToView");
  GLuint view_to_world_uniform_location = shaderNanoSuit.getUniform("viewToWorld");
  GLuint world_to_prespective_uniform_location =
      shaderNanoSuit.getUniform("worldToPrespective");

  // OpenGL features //
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  // glCullFace(GL_FRONT);

  auto prespective_projection{
      glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.001f, 100.0f)};
  auto cam_pos{glm::vec3(0, 0, -1)};
  auto cam_up{glm::vec3(0, 1, 0)};
  auto cam_fwd{glm::vec3(0, 0, 1)};
  glm::dvec2 cursor_pos;
  float keyboard_speed = 0.01;
  while (!glfwWindowShouldClose(window.get())) {
    if (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window.get(), true);

    glfwGetCursorPos(window.get(), &(cursor_pos.x), &(cursor_pos.y));

    auto cam_right{glm::cross(cam_up, cam_fwd)};

    glm::vec3 cam_fwd2(
        glm::rotate(glm::mat4(1.0f),
                    static_cast<float>(cursor_pos.x) * 0.2f * keyboard_speed,
                    -cam_up) *
        glm::rotate(glm::mat4(1.0f),
                    static_cast<float>(cursor_pos.y) * 0.2f * keyboard_speed,
                    cam_right) *
        glm::vec4{cam_fwd, 0.0f});

    auto cam_right_2{glm::normalize(glm::cross(cam_up, cam_fwd2))};

    if (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS)
      cam_pos += cam_fwd2 * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS)
      cam_pos += cam_right_2 * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS)
      cam_pos -= cam_right_2 * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS)
      cam_pos -= cam_fwd2 * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_SPACE) == GLFW_PRESS)
      cam_pos += cam_up * keyboard_speed;
    if (glfwGetKey(window.get(), GLFW_KEY_C) == GLFW_PRESS)
      cam_pos -= cam_up * keyboard_speed;

    // if (glfwGetKey(window.get(), GLFW_KEY_U) == GLFW_PRESS)
    // vertices[3].z += keyboard_speed;
    // if (glfwGetKey(window.get(), GLFW_KEY_J) == GLFW_PRESS)
    // vertices[3].z -= keyboard_speed;
    // glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
    // glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]),
    // vertices.data(), GL_STATIC_DRAW);

    // cam_up = glm::rotate(
    //     glm::mat4(1.0f), static_cast<float>(cursor_pos.x) * keyboard_speed,
    //     -cam_right) * glm::vec4{cam_up, 0.0f};
    // glm::mat4 mvp;
    // mvp[0] = glm::vec4{cam_right, -cam_pos[0]};
    // mvp[1] = glm::vec4{cam_up, -cam_pos[1]};
    // mvp[2] = glm::vec4{cam_fwd, -cam_pos[2]};
    // mvp[3] = glm::vec4{0, 0, 0, 1};
    auto view_transform{glm::lookAt(cam_pos, cam_pos + cam_fwd2, cam_up)};
    auto model_transform{glm::mat4(1.0f)};
    model_transform[3][3] = 10;
    prespective_projection *view_transform *model_transform;

    glUniformMatrix4fv(model_to_view_uniform_location, 1, GL_FALSE,
                       glm::value_ptr(model_transform));
    glUniformMatrix4fv(view_to_world_uniform_location, 1, GL_FALSE,
                       glm::value_ptr(view_transform));
    glUniformMatrix4fv(world_to_prespective_uniform_location, 1, GL_FALSE,
                       glm::value_ptr(prespective_projection));

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glUseProgram(shader.id());
    // glBindVertexArray(vao);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

    // glDrawArrays(GL_TRIANGLES, 0, 3);

    suzzane.draw(shaderNanoSuit);
    // pyramid.draw(shader);

    glfwPollEvents();
    glfwSwapBuffers(window.get());
    // return 0;
  }

  /* CLEAN-UP */
  glDeleteProgram(shader.id());
  glfwTerminate();
  return 0;
}
// cd .. && cd build && make && cd base && ./base && xdotool key , alt+0
