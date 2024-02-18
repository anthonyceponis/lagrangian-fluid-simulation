#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "particle.hpp"

struct PhysicSolver {
  std::vector<Particle> particles;
  glm::vec2 screen_size;
  const uint8_t sub_steps;
  float cell_width;
  uint32_t cell_count_x;
  uint32_t cell_count_y;
  std::vector<std::vector<uint32_t>> grid;

  PhysicSolver(glm::vec2 _screen_size, const float _largest_particle_radius);

  Particle &spawnParticle(glm::vec2 pos, const float radius);

  void update(const float dt);

  void applyGravity();

  void updateParticles(const float dt);

  void constrainParticlesToBoxContainer(const glm::vec2 box_container_size,
                                        const glm::vec2 box_container_center);

  void solveParticleCollisionsBruteForce();

  void solveParticleCollisionsFixedGrid();

  void solveGridCollisionsInRange(const uint32_t x_start, const uint32_t x_end,
                                  const uint32_t y_start, const uint32_t y_end);

  void assignParticlesToFixedGrid();

  void collideTwoParticles(const uint32_t p1_i, const uint32_t p2_i);
};
