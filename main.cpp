#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <glm/glm.hpp>
#include <iostream>

#include "physics/physics.hpp"
// #include "renderer/compute_shader.hpp"
#include "renderer/renderer.hpp"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

float sinFluc(float minSize, float maxSize, float seed) {
  float sizeRange = maxSize - minSize;
  return sizeRange * (0.5f * (float)sin(seed) + 0.5f) + minSize;
}

int main() {
  glm::vec2 screen_size(1200.0f, 800.0f);

  float prev_time = 0.0f;
  float curr_time;
  float dt;

  // GLFW: Init and config
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

  // GLFW: Create window
  GLFWwindow *window = glfwCreateWindow(screen_size.x, screen_size.y,
                                        "Particle Simulation", NULL, NULL);
  if (window == NULL) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  // glfwSwapInterval(0);

  // GLAD: Init
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }

  // Bind callback function to execute on screen resize
  // Gets called on window creation to init viewport
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  const float particle_radius = 4.f;
  const float particle_mass = 2.5f;
  const uint32_t particle_count = 50 * 50;
  const uint8_t sub_steps = 1;
  const float smoothing_radius = 16.f;

  PhysicSolver physic_solver(screen_size, particle_count, particle_radius,
                             particle_mass, sub_steps, smoothing_radius);
  Renderer renderer(physic_solver);

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    // Update delta time
    curr_time = glfwGetTime();
    dt = curr_time - prev_time;
    prev_time = curr_time;
    float fps = 1.0f / dt;

    std::cout << "FPS: " << fps << "\n";

    processInput(window);

    glClearColor(0.9f, 0.9f, 0.9f, 1.0f); // Set the clearing colour
    glClear(GL_COLOR_BUFFER_BIT);         // Use the clearing colour

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
