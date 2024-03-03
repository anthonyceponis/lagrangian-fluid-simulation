#pragma once
#include <glad/glad.h>
#include <string>
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

  ~ComputeShader() { glDeleteProgram(ID); }
};
