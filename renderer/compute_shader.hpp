#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ComputeShader {
public:
  // Program id
  unsigned int ID;

  // Constructor reads and builds the shader
  ComputeShader(const char *cShaderPath) {
    // Retrieve shader source code from files
    // --------------------------------------
    std::string computeCode;
    std::ifstream cShaderFile;

    cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
      // Open files
      cShaderFile.open(cShaderPath);
      std::stringstream cShaderStream;
      // Read file buffer contents into streams
      cShaderStream << cShaderFile.rdbuf();
      // Close file handlers
      cShaderFile.close();
      // Convert streams into strings
      computeCode = cShaderStream.str();
    } catch (std::ifstream::failure e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n";
    }
    const char *cShaderCode = computeCode.c_str();

    // OpenGL shader setup
    // -------------------
    // Compile shader
    unsigned int cShader;
    int success;
    char infoLog[512];

    cShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cShader, 1, &cShaderCode, NULL);
    glCompileShader(cShader);
    glGetShaderiv(cShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(cShader, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER::COMPUTE::COMPILATION_FAILED\n"
                << infoLog << "\n";
    }

    // Shader program
    ID = glCreateProgram();
    glAttachShader(ID, cShader);
    glLinkProgram(ID);
    // Check for linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(ID, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER:PROGRAM:LINKING_FAILED\n" << infoLog << "\n";
    }

    // Clean up - shader already linked to shader program so no longer needed
    glDeleteShader(cShader);
  }

  // Use/activate the shader
  void use() { glUseProgram(ID); }

  template <typename T>
  uint32_t setVector(std::vector<T> &vec, const uint32_t binding_id) {
    uint32_t ssbo;

    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * vec.size(), vec.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_id, ssbo);

    return ssbo;
  }

  void setFloat(const float value, const std::string &name) {
    uint32_t uniform_loc = glGetUniformLocation(this->ID, name.c_str());
    glUniform1f(uniform_loc, value);
  }

  void setUnsignedInt(const uint32_t value, const std::string &name) {
    uint32_t uniform_loc = glGetUniformLocation(this->ID, name.c_str());
    glUniform1ui(uniform_loc, value);
  }

  template <typename T>
  void extractVector(uint32_t ssbo_id, std::vector<T> &desintation) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                       sizeof(T) * desintation.size(), desintation.data());
  }

  void executeSync(const uint32_t work_group_size) {
    // Dispatch workers.
    // Dispatch in multiples of 64 due to 'warp size'.
    glDispatchCompute((work_group_size + 63) / 64, 1, 1);
    // glDispatchCompute(work_group_size, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  ~ComputeShader() { glDeleteProgram(ID); }
};
