#include "renderer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

Renderer::Renderer(PhysicSolver &_solver)
    : solver(_solver), shader("renderer/shaders/circle.vs.glsl",
                              "renderer/shaders/circle.fs.glsl"){};

void Renderer::drawParticles() {
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

  glm::mat4 projection = glm::ortho(0.0f, this->solver.screen_size.x, 0.0f,
                                    this->solver.screen_size.y, -1.0f, 1.0f);
  this->shader.use();
  shader.setMat4("projection", projection);

  glDrawArrays(GL_POINTS, 0, particle_count);
};
