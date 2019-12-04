#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace RenderLoop {
  static auto uploadMatrix(GLuint uniform_location, glm::mat4 mat) -> void {
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(mat));
  }


}
