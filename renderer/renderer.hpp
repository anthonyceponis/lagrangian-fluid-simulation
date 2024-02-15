#pragma once
#include "../physics/physics.hpp"
#include "shader.hpp"

struct Renderer {
  PhysicSolver &solver;
  Shader shader;
  Renderer(PhysicSolver &_solver);
  void drawParticles();
};
