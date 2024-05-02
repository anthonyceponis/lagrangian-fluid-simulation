#include "physics.hpp"
#include "spatial_grid.hpp"

#include <cmath>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>

#include <iostream>

PhysicSolver::PhysicSolver(glm::vec2 _screen_size,
                           const uint32_t _particle_count,
                           const float _particle_radius,
                           const float _particle_mass, const uint8_t _sub_steps,
                           const float _smoothing_radius)
    : particles(_particle_count), world_size(_screen_size),
      sub_steps(_sub_steps), particle_count(_particle_count),
      particle_radius(_particle_radius), particle_mass(_particle_mass),
      smoothing_radius(_smoothing_radius),
      compute_shader("./renderer/shaders/fluid_sim.cs.glsl") {

  // WARNING: particle_count must be square
  const glm::ivec2 spawn_grid_size((int32_t)sqrt(this->particle_count),
                                   (int32_t)sqrt(this->particle_count));
  const float spawn_grid_spacing = 5.f;
  const glm::vec2 spawn_grid_top_left(0.0f, this->world_size.y);

  for (int32_t y = 0; y < spawn_grid_size.y; y++) {
    for (int32_t x = 0; x < spawn_grid_size.x; x++) {
      const uint32_t p_i = y * spawn_grid_size.x + x;
      this->particles.positions[p_i] =
          spawn_grid_top_left +
          glm::vec2((x + 1) * (2 * this->particle_radius + spawn_grid_spacing),
                    -(y + 1) *
                        (2 * this->particle_radius + spawn_grid_spacing));
      this->particles.colours[p_i] = glm::vec3(35.f, 137.f, 218.f) / 255.f;
    }
  }
  this->spatial_grid =
      new SpatialGrid(this->particles.positions, this->smoothing_radius);
}

PhysicSolver::~PhysicSolver() { delete this->spatial_grid; }

void PhysicSolver::update(const float dt) {
  // const float step_dt = dt / this->sub_steps;
  // const float step_dt = (1 / 60.f) / this->sub_steps;
  const float step_dt = 0.0007f;

  for (int32_t i = 0; i < this->sub_steps; i++) {
    // applyGravity(step_dt);

    this->spatial_grid->update();
    // this->calcDensities(step_dt);
    this->calcDensitiesAndApplyPressureForce(step_dt);
    // Integrate
    for (int32_t i = 0; i < this->particle_count; i++) {
      glm::vec2 acc =
          this->particles.forces[i] / this->particles.densities[i].x;
      this->particles.velocities[i] += acc * step_dt;
      this->particles.positions[i] += this->particles.velocities[i] * step_dt;
    }

    this->constrainParticlesToScreen(step_dt);
  }
}

void PhysicSolver::applyGravity(float step_dt) {
  glm::vec2 G(0.0f, -9.81f);
  for (int32_t i = 0; i < this->particle_count; i++) {
    this->particles.velocities[i] += G * step_dt;
  }
}

void PhysicSolver::calcDensities(const float step_dt) {
  const float h = this->smoothing_radius;
  const float pi = 3.14159265f;
  const float POLY6 = 4.f / (pi * glm::pow(h, 8.f));
  for (int32_t i = 0; i < this->particle_count; i++) {
    float density = 0.f;
    for (int32_t j = 0; j < this->particle_count; j++) {
      glm::vec2 rij =
          this->particles.positions[j] - this->particles.positions[i];
      const float r = glm::length(rij);
      if (r < h) {
        density += this->particle_mass * POLY6 * glm::pow(h * h - r * r, 3.f);
      }
    }
    this->particles.densities[i].x = density;
  }
}

void PhysicSolver::calcDensitiesAndApplyPressureForce(const float step_dt) {
  this->compute_shader.use();

  this->compute_shader.setVector(this->particles.positions, 0);
  this->compute_shader.setVector(this->particles.velocities, 1);
  const uint32_t forces_ssbo_id =
      this->compute_shader.setVector(this->particles.forces, 2);
  const uint32_t densities_ssbo_id =
      this->compute_shader.setVector(this->particles.densities, 3);
  this->compute_shader.setVector(this->spatial_grid->spatial_lookup, 4);
  this->compute_shader.setVector(this->spatial_grid->spatial_indicies, 5);

  this->compute_shader.setFloat(step_dt, "dt");
  this->compute_shader.setUnsignedInt(this->particle_count, "particle_count");
  this->compute_shader.setUnsignedInt(
      this->spatial_grid->spatial_lookup.size() - 1, "bucket_count");
  this->compute_shader.setFloat(this->smoothing_radius, "h");
  this->compute_shader.setFloat(this->particle_mass, "particle_mass");
  this->compute_shader.setFloat(300.f, "target_density");
  this->compute_shader.setFloat(2000.f, "pressure_multiplier");
  this->compute_shader.setFloat(3000.f, "near_pressure_multiplier");
  this->compute_shader.setFloat(200.f, "viscosity_strength");

  const uint32_t calc_density_kernel_id = 0;
  const uint32_t apply_fluid_forces_kernel_id = 1;

  // Calculate densities
  this->compute_shader.setUnsignedInt(calc_density_kernel_id, "kernel_id");
  this->compute_shader.executeSync(this->particle_count);

  // Apply fluid forces
  this->compute_shader.setUnsignedInt(apply_fluid_forces_kernel_id,
                                      "kernel_id");
  this->compute_shader.executeSync(this->particle_count);

  // Extract updated vectors
  this->compute_shader.extractVector(forces_ssbo_id, this->particles.forces);
  this->compute_shader.extractVector(densities_ssbo_id,
                                     this->particles.densities);

  for (int32_t i = 0; i < this->particle_count; i++) {
    // std::cout << this->particles.velocities[i].x << " "
    // << this->particles.velocities[i].y << "\n";
    std::cout << this->particles.densities[i].x << "\n";

    // std::cout << this->spatial_grid->spatial_lookup[i] << " ";
  }
  // std::cout << "\n";
}

void PhysicSolver::constrainParticlesToScreen(const float step_dt) {

  const float damp = 0.5f;

  // Right/left
  for (int32_t i = 0; i < this->particle_count; i++) {
    if (this->particles.positions[i].x + this->particle_radius >
        this->world_size.x) {
      this->particles.positions[i].x =
          this->world_size.x - this->particle_radius;
      this->particles.velocities[i].x *= -1 * damp;
    } else if (this->particles.positions[i].x - this->particle_radius < 0.0f) {
      this->particles.positions[i].x = this->particle_radius;
      this->particles.velocities[i].x *= -1 * damp;
    }

    // Top/bottom
    if (this->particles.positions[i].y + this->particle_radius >
        this->world_size.y) {
      this->particles.positions[i].y =
          this->world_size.y - this->particle_radius;
      this->particles.velocities[i].y *= -1 * damp;
    } else if (this->particles.positions[i].y - this->particle_radius < 0.0f) {
      this->particles.positions[i].y = this->particle_radius;
      this->particles.velocities[i].y *= -1 * damp;
    }
  }
}
