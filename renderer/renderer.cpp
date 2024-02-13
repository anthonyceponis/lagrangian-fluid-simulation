#include "renderer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

Renderer::Renderer(PhysicSolver &_solver)
    : solver(_solver), shader("renderer/shaders/circle_using_point.vs",
                              "renderer/shaders/circle_using_point.fs"){};

void Renderer::drawParticles() {
  uint32_t vao, vbo_quad, vbo_instance;
  const uint32_t particle_count = this->solver.particles.size();

  // Quad instance rendering
  // --------------
  // Default radius of 1
  float quad_vertex_data[] = {
      -1.0f, 1.0f,  // TL
      1.0f,  -1.0f, // BR
      1.0f,  1.0f,  // TR

      -1.0f, 1.0f,  // TL
      -1.0f, -1.0f, // BL
      1.0f,  -1.0f, // BR
  };

  // Vertex attribs: [pos_x, pos_y, radius, col_r, col_g, col_b]
  float particle_instance_data[particle_count * 6];

  for (int i = 0; i < particle_count; i++) {
    Particle &p = this->solver.particles[i];
    particle_instance_data[i * 6] = p.pos.x;
    particle_instance_data[i * 6 + 1] = p.pos.y;
    particle_instance_data[i * 6 + 2] = p.radius;
    particle_instance_data[i * 6 + 3] = p.color.r;
    particle_instance_data[i * 6 + 4] = p.color.g;
    particle_instance_data[i * 6 + 5] = p.color.b;
  }

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo_quad);
  glGenBuffers(1, &vbo_instance);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_data), quad_vertex_data,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_instance);
  glBufferData(GL_ARRAY_BUFFER, sizeof(particle_instance_data),
               particle_instance_data, GL_STATIC_DRAW);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)0); // Position
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(2 * sizeof(float))); // Radius
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float))); // Color

  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  glVertexAttribDivisor(1, 1);
  glVertexAttribDivisor(2, 1);
  glVertexAttribDivisor(3, 1);

  // glm::mat4 model(1.0f);
  // glm::mat4 view(1.0f);
  glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

  shader.use();

  // shader.setMat4("model", model);
  // shader.setMat4("view", view);
  shader.setMat4("projection", projection);

  // float default_point_size = 1.0f;
  // glEnable(GL_PROGRAM_POINT_SIZE); // Enable point size control in shader
  // glPointSize(default_point_size);

  // glDrawArrays(GL_POINTS, 0, particle_count);
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particle_count);
};

void Renderer::drawParticlesPoint() {
  const uint32_t particle_count = this->solver.particles.size();

  // Vertex data: [pos_x, pos_y, radius, col_y, col_g, col_b];
  float vertex_data[particle_count * 6];

  for (uint32_t i = 0; i < particle_count; i++) {
    Particle &p = this->solver.particles[i];

    vertex_data[i * 6] = p.pos.x;
    vertex_data[i * 6 + 1] = p.pos.y;
    vertex_data[i * 6 + 2] = p.radius;
    vertex_data[i * 6 + 3] = p.color.r;
    vertex_data[i * 6 + 4] = p.color.g;
    vertex_data[i * 6 + 5] = p.color.b;
  }

  uint32_t vao, vbo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  float default_point_size = 10.0f;
  glEnable(GL_PROGRAM_POINT_SIZE); // Enable point size control in shader
  glPointSize(default_point_size);

  glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
  this->shader.use();
  shader.setMat4("projection", projection);

  glDrawArrays(GL_POINTS, 0, particle_count);
};
