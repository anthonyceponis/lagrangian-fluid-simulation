#include "particle.hpp"
#include <iostream>

// WARNING: force must be set to zero due to weird bug where fifth spawned
// particle produces crazy forces values for unknown reason.
Particle::Particle(glm::vec2 _pos, float _radius)
    : pos(_pos), prev_pos(_pos), radius(_radius), mass(_radius),
      force(glm::vec2(0.0f, 0.0f)) {}

void Particle::update(float dt) {
  // Current velocity is difference between positions (kind of). Actual velocity
  // would require dividing by dt as well.
  glm::vec2 vel = this->pos - this->prev_pos;

  // The current position becomes the old one
  this->prev_pos = this->pos;

  // F = ma
  glm::vec2 acc = this->force;

  // DEBUGGING
  // ---------
  // std::cout << vel.x << " " << vel.y << " " << acc.x << " " << acc.y << " "
  // << dt * dt << "\n";

  // New position via verlet integration
  this->pos += vel + acc * dt * dt;

  // Reset force
  this->force = glm::vec2(0.0f, 0.0f);
}
