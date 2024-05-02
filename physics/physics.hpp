#pragma once

#include <cstdint>

#include <glm/glm.hpp>

// #include "gpu_compute.hpp"
#include "particles.hpp"
#include "spatial_grid.hpp"
#include "../renderer/compute_shader.hpp"

struct PhysicSolver {
  Particles particles;
  glm::vec2 world_size;
  const uint8_t sub_steps;
  uint32_t particle_count;
  float particle_radius;
  float particle_mass;
  float smoothing_radius;
  SpatialGrid *spatial_grid;
  ComputeShader compute_shader;

  PhysicSolver(glm::vec2 _screen_size, const uint32_t _particle_count,
               const float _particle_radius, const float _particle_mass,
               const uint8_t _sub_steps, const float _smoothing_radius);

  ~PhysicSolver();

  void update(const float dt);

  void applyGravity(float step_dt);

  void calcDensities(const float step_dt);

  void calcDensitiesAndApplyPressureForce(const float step_dt);

  void constrainParticlesToScreen(const float step_dt);
};
