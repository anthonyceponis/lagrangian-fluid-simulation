#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdint>
#include <glm/glm.hpp>
#include <iostream>

#include "physics/physics.hpp"
#include "renderer/renderer.hpp"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

float sinFluc(float minSize, float maxSize, float seed) {
  float sizeRange = maxSize - minSize;
  return sizeRange * (0.5f * (float)sin(seed) + 0.5f) + minSize;
}

int main() {
  glm::vec2 screen_size(800.0f, 600.0f);

  const float PI = 3.14159265f;

  float prev_time = 0.0f;
  float curr_time;
  float dt;

  // GLFW: Init and config
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // GLFW: Create window
  GLFWwindow *window = glfwCreateWindow(screen_size.x, screen_size.y,
                                        "Particle Simulation", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // GLAD: Init
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD\n";
    return -1;
  }

  // Bind callback function to execute on screen resize
  // Gets called on window creation to init viewport
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  PhysicSolver physic_solver(glm::vec2(800.0f, 600.0f));
  Renderer renderer(physic_solver);
  const uint32_t particle_count = 500;
  uint32_t active_particles = 0;
  float timer = 0.0f;

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    // Update delta time
    curr_time = glfwGetTime();
    dt = curr_time - prev_time;
    prev_time = curr_time;
    float fps = 1.0f / dt;

    std::cout << "FPS: " << fps << " Active particles: " << active_particles
              << "\n";

    processInput(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set the clearing colour
    glClear(GL_COLOR_BUFFER_BIT);         // Use the clearing colour

    timer += dt;
    if (fps > 50.0f && timer >= 0.01f) {
      float radius = sinFluc(3.0f, 3.0f, curr_time * 2);
      glm::vec2 center(screen_size.x / 10.0f, 9.0f / 10.0f * screen_size.y);
      glm::ivec3 color(sinFluc(0.0f, 255.0f, curr_time),
                       sinFluc(0.0f, 255.0f, curr_time + 0.33f * 2 * PI),
                       sinFluc(0.0f, 255.0f, curr_time + 0.66f * 2 * PI));
      glm::vec2 vel(100.0f, 0);
      Particle &p = physic_solver.spawnParticle(center, radius);
      p.color = color;
      p.prev_pos = p.pos - (vel * dt);

      active_particles++;
      timer = 0.0f;
    }
    physic_solver.update(dt);
    // renderer.drawParticles();
    renderer.drawParticlesPoint();

    glfwSwapBuffers(window); // Double buffering: swap current OpenGL colour
                             // buffer with the screen buffer to update screen
                             // pixels with computed pixel values
    glfwPollEvents(); // Check for input events (e.g. keypresses, window close
                      // press) and calls respective callback functions to
                      // update window state.
  }

  // Clean up
  glfwTerminate();
  return 0;
}

// Callback function to reset OpenGL viewport on screen resize
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

// Input handling function
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

// Command to run program: g++ main.cpp glad.c -ldl-lglfw -pthread
