#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hh" 
#include "utils.hh"
#include "light.hh"
#include "model.hh"
#include "shader.hh"
#include "overlay.hh"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>
namespace fs = std::filesystem;

std::function<void(GLFWwindow*, double, double)> g_callback_mouse;

int main()
{
  using namespace std::string_literals;

  constexpr int window_height{1366}, window_width{768};
  // std::setlocale(LC_ALL, "POSIX");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  Utils::GLFWwindowUniquePtr window{
      glfwCreateWindow(window_height, window_width, "Shadow Mapping Example OpenGL", nullptr, nullptr)};
  glfwMakeContextCurrent(window.get());
  if (!window.get())
    throw std::runtime_error{"glfw window create failed"};
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
    throw std::runtime_error{"GLAD init failed"};

  // Adjust viewport upon window resize
  glfwSetFramebufferSizeCallback(
      window.get(), [](auto, int w, int h) { glViewport(0, 0, w, h); });
  glfwSetScrollCallback(window.get(), [](auto, auto, auto) {});

  // Grab cursor
  glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  const auto base_path{fs::current_path() / "../../"};
  Shader shader_nanosuit, shader_shadowmap, shader_depthmap_overlay;
  shader_shadowmap
      .attach(base_path / "shader/shadow_mapping/shadow.vert", GL_VERTEX_SHADER)
      .attach(base_path / "shader/shadow_mapping/shadow.frag",
              GL_FRAGMENT_SHADER)
      .link();
  shader_nanosuit
      .attach(base_path /
                  "shader/shadow_mapping/texturewithshadow/texshad.vert",
              GL_VERTEX_SHADER)
      .attach(base_path /
                  "shader/shadow_mapping/texturewithshadow/texshad.frag",
              GL_FRAGMENT_SHADER)
      .link();
  shader_depthmap_overlay
      .attach(base_path / "shader/shadow_mapping/renderdepthmap/depth.vert", GL_VERTEX_SHADER)
      .attach(base_path / "shader/shadow_mapping/renderdepthmap/depth.frag", GL_FRAGMENT_SHADER)
      .link();

  Model nanosuit{base_path / "res/nanosuit/nanosuit.obj"};
  Model cube{base_path / "res/cube/cube.obj"};

  Light directional_light;

  FPSCamera camera{
      glm::vec3(0, 1.5, 2), // cam_pos
      glm::vec3(0, 1, 0),   // cam_up
      window_height / static_cast<float>(window_width),  // aspect-ratio
      -90.0f,              // yaw
      -18.0f,              // pitch
      glm::radians(60.0f), // FOV
  };
  g_callback_mouse = [&camera](GLFWwindow *w, double x, double y) {   /* Mouse Support */
    camera.mouseCallback(w, x, y);
  }; // HACK
  glfwSetCursorPosCallback(window.get(), [](GLFWwindow *w, double x, double y) {
    g_callback_mouse(w, x, y);
  });
  camera.movementSpeed *= 10;

  Overlay depthmap{camera.aspect_ratio};
  bool should_render_depthmap_overlay = true;

  //*OpenGL features */
  glEnable(GL_CULL_FACE);

  glClearColor(0.227451f, 0.227451f, 0.227451f, 1.0f);
  auto last_frame{std::chrono::high_resolution_clock::now()};
  auto current_frame{std::chrono::high_resolution_clock::now()};
  auto delta_time{current_frame - last_frame};
  constexpr int TARGET_FPS = 60;
  constexpr float OPTIMAL_TIME{1e9 / TARGET_FPS};
  do {
    if (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window.get(), true);
    if (glfwGetKey(window.get(), GLFW_KEY_M) == GLFW_PRESS)
        should_render_depthmap_overlay ^= 1;

    camera.renderloopUpdateView(window, delta_time.count() * 1e-9);

    /* BEGIN RENDER */
    // 1. Render to depth map from light's point of view
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, directional_light.depthMapFBO);
    glUseProgram(shader_shadowmap.id());
    auto render_depthmap_lambda{[&nanosuit] {
      glCullFace(GL_FRONT); // peter panning
    //   glClear(GL_DEPTH_BUFFER_BIT);  // linux mesa doesn't need it
      nanosuit.drawWihtoutTextureBinding();
      glCullFace(GL_BACK);
    }};
    directional_light.render(shader_shadowmap, render_depthmap_lambda);

    // 2. Render scene as normal with shadow mapping using depth map
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_nanosuit.id());
    glUniformMatrix4fv(shader_nanosuit.getUniform("prespective"), 1,
                       GL_FALSE, glm::value_ptr(camera.prespective_matrix));
    glUniformMatrix4fv(shader_nanosuit.getUniform("view"), 1, GL_FALSE,
                       glm::value_ptr(camera.view_matrix));
    glUniformMatrix4fv(shader_nanosuit.getUniform("model"), 1, GL_FALSE,
                       glm::value_ptr(glm::mat4(1.0f)));
    glUniformMatrix4fv(shader_nanosuit.getUniform("lightSpaceMatrix"s), 1,
                       GL_FALSE,
                       glm::value_ptr(directional_light.depthProjectionMatrix *
                                      directional_light.depthViewMatrix));

    glm::vec4 light_position =
        directional_light.depthViewMatrix * glm::vec4(1.0f);
    glUniform3f(shader_nanosuit.getUniform("light_position"s),
                light_position.x / light_position.w,
                light_position.y / light_position.w,
                light_position.z / light_position.w);
    glUniform3f(shader_nanosuit.getUniform("viewPos"s), camera.cameraPos.x,
                camera.cameraPos.y, camera.cameraPos.z);

    static auto shadowmap_uniform_location =
        shader_nanosuit.getUniform("shadowMap"s);
    glActiveTexture(GL_TEXTURE0); // First tex unit is used for shadowMap texture.
    glUniform1i(shadowmap_uniform_location, 0);
    glBindTexture(GL_TEXTURE_2D, directional_light.depthTexture);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
    nanosuit.draw(shader_nanosuit, 1u);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // unwireframe
    
    glActiveTexture(GL_TEXTURE2); // disable specular map
    glBindTexture(GL_TEXTURE_2D, 0);
    cube.draw(shader_nanosuit, 1u);
    
    if (should_render_depthmap_overlay) {
      glUseProgram(shader_depthmap_overlay.id());
      glDisable(GL_DEPTH_TEST);
      glBindVertexArray(depthmap.VAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, directional_light.depthTexture);
      depthmap.draw();
    }

    glfwSwapBuffers(window.get());
    glfwPollEvents();

    current_frame = std::chrono::high_resolution_clock::now();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    std::this_thread::sleep_for(
        std::chrono::duration<float, std::nano>(
            std::max(0.0, OPTIMAL_TIME - delta_time.count() * 1e-9)));
  } while (!glfwWindowShouldClose(window.get()));

  /* CLEAN-UP */
  directional_light.destory();
  cube.destory();
  nanosuit.destory();
  depthmap.destory();
  shader_nanosuit.destory();
  shader_shadowmap.destory();
  shader_depthmap_overlay.destory();
  glfwTerminate();
  return 0;
}
