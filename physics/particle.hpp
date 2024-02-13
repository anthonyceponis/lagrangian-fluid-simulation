#pragma once

#include <glm/glm.hpp>

struct Particle {
  float radius;
  float mass; // All particles have same density so mass is just the radius.
  glm::vec2 pos;
  glm::vec2 prev_pos;
  glm::ivec3 color;
  glm::vec2 force;

  Particle(glm::vec2 _pos, float _radius);

  void update(float dt);
};
