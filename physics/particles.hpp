#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Particles {
  // Misc
  const uint32_t particle_count;

  // Physics
  std::vector<glm::vec2> positions;
  // std::vector<glm::vec2> prev_positions;
  // std::vector<glm::vec2> proj_positions;
  std::vector<glm::vec2> velocities;
  std::vector<glm::vec2> forces;
  std::vector<glm::vec2> densities;

  // Appearance
  std::vector<glm::vec3> colours;

  Particles(const uint32_t _particle_count);
};
