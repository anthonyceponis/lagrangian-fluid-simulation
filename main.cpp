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
  glm::vec2 screen_size(1200.0f, 800.0f);

  const float PI = 3.14159265f;

  float prev_time = 0.0f;
  float curr_time;
  float dt;

  // GLFW: Init and config
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

  // GLFW: Create window
  GLFWwindow *window = glfwCreateWindow(screen_size.x, screen_size.y,
                                        "Particle Simulation", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  // glfwSwapInterval(0);

  // GLAD: Init
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD\n";
    return -1;
  }

  // Bind callback function to execute on screen resize
  // Gets called on window creation to init viewport
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  const float max_radius = 5.0f;
  const float min_radius = 5.0f;
  const uint32_t particle_count = 5000;
  PhysicSolver physic_solver(screen_size, max_radius);
  Renderer renderer(physic_solver);
  uint32_t active_particles = 0;
  const uint32_t particle_spawn_rate = 8;
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
    if (fps > 55.0f && timer >= 0.05f) {
      float radius = sinFluc(min_radius, max_radius, curr_time * 1);
      glm::vec2 vel(175.0f, 0);
      glm::ivec3 color(sinFluc(0.0f, 255.0f, curr_time),
                       sinFluc(0.0f, 255.0f, curr_time + 0.33f * 2 * PI),
                       sinFluc(0.0f, 255.0f, curr_time + 0.66f * 2 * PI));

      for (uint32_t i = 0; i < particle_spawn_rate; i++) {
        glm::vec2 center(50.0, screen_size.y - 20.0f - (max_radius * 3 * i));
        Particle &p = physic_solver.spawnParticle(center, radius);
        p.color = color;
        p.prev_pos = p.pos - (vel * dt);
      }

      active_particles += particle_spawn_rate;
      timer = 0.0f;
    }
    physic_solver.update(dt);
    renderer.drawParticles();

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
