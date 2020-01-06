#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.hh>

#include <string>
#include <iostream>
#include <functional>

class Light {
public:
  // glm::vec3 lightInvDir;
  glm::mat4 depthProjectionMatrix;
  glm::mat4 depthViewMatrix;
  glm::mat4 depthModelMatrix;
  // glm::mat4 depthMVP;

  GLuint depthMapFBO;
  GLuint depthTexture;

  static constexpr int SHADOW_HEIGHT = 1024;
  static constexpr int SHADOW_WIDTH  = 1024;

  // lightInvDir <- glm::vec3(0.5f,2,2)
  Light():
    Light{glm::ortho<float>(-10,10,-10,10,-10,20),
          glm::lookAt(glm::vec3(0.5f,2,2), glm::vec3(0,0,0), glm::vec3(0,1,0)), // Not correct
          glm::mat4{1.0f}}
  {}
  Light(// const glm::vec3& lightInvDir,
        const glm::mat4& depthProjectionMatrix,
        const glm::mat4& depthViewMatrix,
        const glm::mat4& depthModelMatrix):
    depthProjectionMatrix{depthProjectionMatrix},
    depthViewMatrix{depthViewMatrix},
    depthModelMatrix{depthModelMatrix} {
      if(!this->setupDepthTexture())
        std::cerr << "ERROR::light::render::setupDepth -> returned false."
                  << std::endl;
    }

    auto destory() -> void {
      glDeleteTextures(1, &depthMapFBO);
      glDeleteFramebuffers(1, &depthMapFBO);
    }

  // auto calcDepthMVP() -> glm::mat4 const {
  //   return depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
  // }

  auto bindDepthMap() -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  }

  auto render(Shader &shaderRenderToDepthMap,
              std::function<void()> subrenderToDepthMap) {
    using namespace std::string_literals;
    GLint viewport[4];

    // Render to depth map from light's point of view
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);    // set viewport proportions
    glClear(GL_DEPTH_BUFFER_BIT);

    auto light_space_matrix_uniform_location{shaderRenderToDepthMap.getUniform("lightSpaceMatrix"s)};
    glUniformMatrix4fv(light_space_matrix_uniform_location,
                       1, GL_FALSE, glm::value_ptr(depthViewMatrix));
    auto model_matrix_uniform_location{shaderRenderToDepthMap.getUniform("lightProjection"s)};
    glUniformMatrix4fv(model_matrix_uniform_location,
                       1, GL_FALSE, glm::value_ptr(depthProjectionMatrix));

    subrenderToDepthMap();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  }

  auto setupDepthTexture() -> bool {
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    // Depth texutre is slower than depth buffer, but you can
    // sample it later in fragment shader.
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,                    // mipmap level
                 GL_DEPTH_COMPONENT16, // internal format
                 SHADOW_WIDTH,
                 SHADOW_HEIGHT,
                 0,                    // border
                 GL_DEPTH_COMPONENT,   // format of pixel data
                 GL_FLOAT,             // ^'s type
                 nullptr);             // pointer to data
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    std::array<float, 4> borderColor{1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor.data());
    glFramebufferTexture(GL_FRAMEBUFFER,
                         GL_DEPTH_ATTACHMENT, // attachment
                         depthTexture,        // texture
                         0);                  // mipmap level
    glDrawBuffer(GL_NONE); // Don't draw to any color buffer.
    glReadBuffer(GL_NONE);
    // Always check if framebuffer is ok.
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      return false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
  }

};
