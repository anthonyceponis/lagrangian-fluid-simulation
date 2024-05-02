#include "particles.hpp"

Particles::Particles(const uint32_t _particle_count)
    : particle_count(_particle_count), positions(_particle_count),
      velocities(_particle_count), forces(_particle_count),
      densities(_particle_count), colours(_particle_count){};
