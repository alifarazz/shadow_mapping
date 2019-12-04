#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.hh>

#include <string>
#include <iostream>
#include <functional>

// TODO: send MVP mats indivually to gpu
class Light
{
public:
  // glm::vec3 lightInvDir;
  glm::mat4 depthProjectionMatrix;
  glm::mat4 depthViewMatrix;
  glm::mat4 depthModelMatrix;
  glm::mat4 depthMVP;
  glm::mat4 depthMVP2;


  GLuint depthMapFBO;
  GLuint depthTexture;

  static constexpr int SHADOW_HEIGHT = 1024;
  static constexpr int SHADOW_WIDTH  = 1024;

  // lightInvDir <- glm::vec3(0.5f,2,2)
  Light():
    depthProjectionMatrix{glm::ortho<float>(-10,10,-10,10,-10,20)},
    depthViewMatrix{glm::lookAt(glm::vec3(0.5f,2,2), glm::vec3(0,0,0), glm::vec3(0,1,0))},
    depthModelMatrix{glm::mat4{1.0}}
  {setupLight();}
  Light(// const glm::vec3& lightInvDir,
        const glm::mat4& depthProjectionMatrix,
        const glm::mat4& depthViewMatrix,
        const glm::mat4& depthModelMatrix):
    // lightInvDir{lightInvDir},
    depthProjectionMatrix{depthProjectionMatrix},
    depthViewMatrix{depthViewMatrix},
    depthModelMatrix{depthModelMatrix}
  {setupLight();}

  ~Light() {
    // glDeleteFramebuffers(1, &depthMapFBO);
    // glDeleteTextures(1, &depthTexture);
  }

  auto setupLight() -> void {
    depthMVP = this->calcDepthMVP();
    if(!this->setupDepthTexture())
      std::clog << "ERROR::light::render::setupDepth -> returned false."
                << std::endl;
}

  auto calcDepthMVP() -> glm::mat4 const {
    return depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
  }

  auto render(Shader &shaderRenderToDepthMap,
              std::function<void(Shader&)> subrenderToDepthMap,
              Shader &shaderRenderTheDepthMap,
              std::function<void()> subrenderTheDepthMap) {
    using namespace std::string_literals;
    GLint viewport[4];

    glUseProgram(shaderRenderToDepthMap.id());
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // 1. Render to depth map from light's point of view
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);    // set viewport proportions
    glClear(GL_DEPTH_BUFFER_BIT);

    auto depth_matrix_uniform_location{shaderRenderToDepthMap.getUniform("depthMVP"s)};
    glUniformMatrix4fv(depth_matrix_uniform_location,
                       1, GL_FALSE, glm::value_ptr(depthMVP));
    subrenderToDepthMap(shaderRenderToDepthMap);

    // 2. Render scene as normal with shadow mapping using depth map
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shaderRenderTheDepthMap.id());
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    depth_matrix_uniform_location = shaderRenderTheDepthMap.getUniform("depthMVP"s);
    glUniformMatrix4fv(depth_matrix_uniform_location,
                       1, GL_FALSE, glm::value_ptr(depthMVP2));
    // auto depthMapTexID = shaderRenderTheDepthMap.getUniform("depthMapTexture"s);
    // glActiveTexture(GL_TEXTURE0);
    // glUniform1i(depthMapTexID, 0);
    glBindTexture(GL_TEXTURE_2D, depthTexture); // it only has the depth texture.
    subrenderTheDepthMap();
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
