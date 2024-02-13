#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
  // Program id
  unsigned int ID;

  // Constructor reads and builds the shader
  Shader(const char *vShaderPath, const char *fShaderPath) {
    // Retrieve shader source code from files
    // --------------------------------------
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
      // Open files
      vShaderFile.open(vShaderPath);
      fShaderFile.open(fShaderPath);
      std::stringstream vShaderStream, fShaderStream;
      // Read file buffer contents into streams
      vShaderStream << vShaderFile.rdbuf();
      fShaderStream << fShaderFile.rdbuf();
      // Close file handlers
      vShaderFile.close();
      fShaderFile.close();
      // Convert streams into strings
      vertexCode = vShaderStream.str();
      fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n";
    }
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // OpenGL shader setup
    // -------------------
    // Compile shaders
    unsigned int vShader, fShader;
    int success;
    char infoLog[512];

    // Vertex shader
    vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderCode, NULL);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vShader, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER::VERTEX::COMPILATION_FAILED\n"
                << infoLog << "\n";
    }
    // Fragment shader
    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderCode, NULL);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fShader, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER:FRAGMENT::COMPILATION_FAILED\n"
                << infoLog << "\n";
    }

    // Shader program
    ID = glCreateProgram();
    glAttachShader(ID, vShader);
    glAttachShader(ID, fShader);
    glLinkProgram(ID);
    // Check for linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(ID, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER:PROGRAM:LINKING_FAILED\n" << infoLog << "\n";
    }

    // Clean up - shaders already linked to shader program so no longer needed
    glDeleteShader(vShader);
    glDeleteShader(fShader);
  }
  // Use/activate the shader
  void use() { glUseProgram(ID); }
  // Utility uniform functions
  void setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }
  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }
  void setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }

  void setVec3i(const std::string &name, glm::ivec3 &v) {
    unsigned int location = glGetUniformLocation(ID, name.c_str());
    glUniform3i(location, v.x, v.y, v.z);
  }

  void setMat4(const std::string &name, glm::mat4 &matrix) const {
    unsigned int location = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
  }

  ~Shader() { glDeleteProgram(ID); }
};
