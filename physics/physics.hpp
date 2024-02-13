#pragma once

#include <cstdint>
#include <vector>
// #include <thread>

#include <glm/glm.hpp>

#include "particle.hpp"

struct PhysicSolver {
  std::vector<Particle> particles;
  glm::vec2 screen_size;
  const uint8_t sub_steps;

  PhysicSolver(glm::vec2 _screen_size);

  Particle &spawnParticle(glm::vec2 pos, const float radius);

  void update(const float dt);

  void applyGravity();

  void updateParticles(const float dt);

  void constrainParticlesToBoxContainer(const glm::vec2 box_container_size,
                                        const glm::vec2 box_container_center);

  void constrainParticlesToCircleContainer(const float circle_radius,
                                           const glm::vec2 circle_center);

  void solveParticleCollisionsBruteForce();

  void solveParticleCollisionsFixedGrid();

  void assignParticlesToFixedGrid(
      std::vector<std::vector<std::vector<uint32_t>>> &grid,
      uint32_t cell_count_x, uint32_t cell_count_y, float cell_width);

  void collideTwoParticles(Particle &p1, Particle &p2);
};
