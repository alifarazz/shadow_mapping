#include <GLFW/glfw3.h>
#include <glad/glad.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.hh"
#include "shader.hh"
#include "light.hh"
#include "glfwwindow.hh"
#include "camera.hh"

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

std::function<void(GLFWwindow *, double, double)> mouseCallbackStub;

int main()
{
  int windowHeight{1366}, winWidth{768};

  // std::setlocale(LC_ALL, "POSIX");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  GLFWwindowUniquePtr window{
      glfwCreateWindow(windowHeight, winWidth, "", nullptr, nullptr)};
  glfwMakeContextCurrent(window.get());
  if (!window.get())
    throw std::runtime_error{"glfw window create failed"};
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
    throw std::runtime_error{"GLAD init failed"};

  // Adjust viewport upon window resize
  glfwSetFramebufferSizeCallback(
      window.get(), [](auto, int w, int h) { glViewport(0, 0, w, h); });
  glfwSetScrollCallback(window.get(), [](auto, auto, auto){});

  // Grab cursor
  glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  /* SHADERS */
  Shader shaderNanoSuit{}, shaderRenderDepthMap{}, shaderShowDepthMap{};
  // shader.attach(fs::current_path() / "shader/empty.vert", GL_VERTEX_SHADER)
  //     .attach(fs::current_path() / "shader/mono.frag", GL_FRAGMENT_SHADER)
  //     .link();
  shaderNanoSuit
      .attach(fs::current_path() / "shader/lambertian.vert", GL_VERTEX_SHADER)
      .attach(fs::current_path() / "shader/texture.frag", GL_FRAGMENT_SHADER)
      .link();
  shaderRenderDepthMap
    .attach(fs::current_path() / "shader/shadow_mapping/shadow.vert", GL_VERTEX_SHADER)
    .attach(fs::current_path() / "shader/shadow_mapping/shadow.frag", GL_FRAGMENT_SHADER)
    .link();
  shaderShowDepthMap
    .attach(fs::current_path() / "shader/shadow_mapping/renderdepthmap/renderdepth.vert", GL_VERTEX_SHADER)
    .attach(fs::current_path() / "shader/shadow_mapping/renderdepthmap/renderdepth.frag", GL_FRAGMENT_SHADER)
    .link();

  /* LOADING MODELS */
  // Model pyramid{fs::current_path() / fs::path("res/suzzane/Pyramid.obj")};
  // Model suzzane{fs::current_path() / fs::path("res/suzzane/suzzane.glb")};
  Model nanosuit{fs::current_path() / fs::path("res/nanosuit/nanosuit.obj")};

  /* LIGHTS */
  Light directionalLight{};

  //*OpenGL features */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  // glCullFace(GL_FRONT);

  auto cam_up{glm::vec3(0, 1, 0)};
  auto cam_pos{glm::vec3(0, 1, -1)};
  auto cam_fwd{glm::normalize(glm::vec3(0, 1, -1))};
  FPSCamera camera{cam_pos,
                   cam_fwd,
                   cam_up,
                   windowHeight / float(winWidth)};
  mouseCallbackStub = [&camera](GLFWwindow *w, double x, double y){camera.mouseCallback(w, x, y);};
  glfwSetCursorPosCallback(window.get(), [](GLFWwindow *w, double x, double y){mouseCallbackStub(w, x, y);});


  glClearColor(0.227451f, 0.227451f, 0.227451f, 1.0f);
  float deltaTime, lastFrame;
  while (!glfwWindowShouldClose(window.get())) {
    if (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window.get(), true);
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    camera.renderloopUpdateMVP(window, deltaTime);
    camera.model_transform[3][3] = 10; // down-scale everything by 10

    /* BEGIN RENDER */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    directionalLight.depthMVP2 = camera.prespective_transform * camera.view_transform * camera.model_transform;
    directionalLight.render(shaderRenderDepthMap,
                            [&](Shader &shader){nanosuit.draw(shader);},
                            shaderShowDepthMap,
                            [&]{nanosuit.drawWihtoutTextureBinding();});

    // glUseProgram(shaderNanoSuit.id());
    // camera.uploadMVP(shaderNanoSuit.getUniform("modelToView"),
    //                  shaderNanoSuit.getUniform("viewToWorld"),
    //                  shaderNanoSuit.getUniform("worldToPrespective"));
    // nanosuit.draw(shaderNanoSuit);

    glfwPollEvents();
    glfwSwapBuffers(window.get());
  }

  /* CLEAN-UP */
  // glDeleteFramebuffers(1, &directionalLight.depthMapFBO);
  // glDeleteTextures(1, &directionalLight.depthTexture);
  glDeleteProgram(shaderNanoSuit.id());
  glDeleteProgram(shaderRenderDepthMap.id());
  glDeleteProgram(shaderShowDepthMap.id());
  glfwTerminate();
  return 0;
}
