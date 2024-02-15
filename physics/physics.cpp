#include "physics.hpp"

#include <glm/glm.hpp>
#include <iostream>

PhysicSolver::PhysicSolver(glm::vec2 _screen_size,
                           const uint32_t _largest_particle_radius)
    : screen_size(_screen_size), sub_steps(8),
      largest_particle_radius(_largest_particle_radius) {}

Particle &PhysicSolver::spawnParticle(glm::vec2 pos, const float radius) {
  this->particles.emplace_back(pos, radius);

  return this->particles.back();
}

void PhysicSolver::update(const float dt) {
  const float step_dt = dt / this->sub_steps;

  for (int i = 0; i < this->sub_steps; i++) {
    this->applyGravity();
    this->updateParticles(step_dt);
    this->constrainParticlesToBoxContainer(screen_size, screen_size / 2.0f);
    this->solveParticleCollisionsFixedGridSpatialHash();
    // this->solveParticleCollisionsFixedGridLoose();
    // this->solveParticleCollisionsFixedGrid();
    // this->solveParticleCollisionsBruteForce();
  }
}

void PhysicSolver::applyGravity() {
  const float g = 500.0f;
  for (Particle &p : this->particles) {
    p.force += glm::vec2(0, -g);
  }
}

void PhysicSolver::updateParticles(const float dt) {
  for (Particle &p : this->particles) {
    p.update(dt);
  }
}

void PhysicSolver::constrainParticlesToBoxContainer(
    const glm::vec2 box_container_size, const glm::vec2 box_container_center) {
  const float e = 0.5f;
  const glm::vec2 box_container_half_size = box_container_size / 2.0f;

  const float box_left = box_container_center.x - box_container_half_size.x;
  const float box_right = box_container_center.x + box_container_half_size.x;
  const float box_top = box_container_center.y + box_container_half_size.y;
  const float box_bottom = box_container_center.y - box_container_half_size.y;

  for (Particle &p : this->particles) {
    if (p.pos.x + p.radius > box_right) {
      const float original_displacement_x = p.pos.x - p.prev_pos.x;
      p.pos.x -= 2 * (p.pos.x + p.radius - box_right);
      p.prev_pos.x = p.pos.x + e * original_displacement_x;
    } else if (p.pos.x - p.radius < box_left) {
      const float original_displacement_x = p.prev_pos.x - p.pos.x;
      p.pos.x += 2 * (box_left - (p.pos.x - p.radius));
      p.prev_pos.x = p.pos.x - e * original_displacement_x;
    }
    if (p.pos.y + p.radius > box_top) {
      const float original_displacement_y = p.pos.y - p.prev_pos.y;
      p.pos.y -= 2 * (p.pos.y + p.radius - box_top);
      p.prev_pos.y = p.pos.y + e * original_displacement_y;
    } else if (p.pos.y - p.radius < box_bottom) {
      const float original_displacement_y = p.prev_pos.y - p.pos.y;
      p.pos.y += 2 * (box_bottom - (p.pos.y - p.radius));
      p.prev_pos.y = p.pos.y - e * original_displacement_y;
    }
  }
}

void PhysicSolver::constrainParticlesToCircleContainer(
    const float circle_radius, const glm::vec2 circle_center) {
  for (Particle &p : this->particles) {
    const float distanceFromCenter = glm::length(p.pos - circle_center);
    const float ratio = (circle_radius - p.radius) / distanceFromCenter;

    // If the particle is outside of the container
    if (ratio < 1.0f) {
      p.pos -= circle_center;
      p.pos *= ratio;
      p.pos += circle_center;
    }
  }
}

void PhysicSolver::solveParticleCollisionsBruteForce() {
  for (uint32_t i = 0; i < this->particles.size(); i++) {
    for (uint32_t j = i + 1; j < this->particles.size(); j++) {
      this->collideTwoParticles(i, j);
    }
  }
}

void PhysicSolver::solveParticleCollisionsFixedGrid() {
  const float cell_width =
      2 * this->largest_particle_radius; // Cells are squares.
  const uint32_t cell_count_x = std::ceil(this->screen_size.x / cell_width);
  const uint32_t cell_count_y = std::ceil(this->screen_size.y / cell_width);

  // Stores particle args from main particles array in a flattened row major
  // matrix.
  std::vector<std::vector<uint32_t>> grid(cell_count_y * cell_count_x);

  this->assignParticlesToFixedGrid(grid, cell_count_x, cell_count_y,
                                   cell_width);

  for (uint32_t y = 0; y < cell_count_y; y++) {
    for (uint32_t x = 0; x < cell_count_x; x++) {
      const uint32_t cell_particle_count = grid[y * cell_count_x + x].size();
      for (uint32_t p1_i = 0; p1_i < cell_particle_count; p1_i++) {
        for (uint32_t p2_i = p1_i + 1; p2_i < cell_particle_count; p2_i++) {
          // p1_i and p2_i store the indices relative to the vector in the cell.
          // But we need to convert the indices relative to the particles array.
          this->collideTwoParticles(grid[y * cell_count_x + x][p1_i],
                                    grid[y * cell_count_x + x][p2_i]);
        }
      }
    }
  }
}

void PhysicSolver::solveParticleCollisionsFixedGridLoose() {
  const float cell_width =
      2 * this->largest_particle_radius; // Cells are squares.
  // WARNING: Grid is padded.
  const uint32_t cell_count_x = std::ceil(this->screen_size.x / cell_width) + 2;
  const uint32_t cell_count_y = std::ceil(this->screen_size.y / cell_width) + 2;

  // Stores particle args from main particles array.
  // Add padding around grid to avoid stupid boundary checking.
  std::vector<std::vector<uint32_t>> grid(cell_count_y * cell_count_x);

  this->assignParticlesToFixedGridLoose(grid, cell_count_x, cell_count_y,
                                        cell_width);

  for (int32_t y = 1; y < cell_count_y - 1; y++) {
    for (int32_t x = 1; x < cell_count_x - 1; x++) {
      const uint32_t middle_cell_particle_count =
          grid[y * cell_count_x + x].size();

      for (int32_t x_offset = -1; x_offset <= 1; x_offset++) {
        for (int32_t y_offset = -1; y_offset <= 1; y_offset++) {
          const uint32_t neighbour_cell_particle_count =
              grid[(y + y_offset) * cell_count_x + x + x_offset].size();
          for (int32_t p1_i = 0; p1_i < middle_cell_particle_count; p1_i++) {
            for (int32_t p2_i = 0; p2_i < neighbour_cell_particle_count;
                 p2_i++) {
              this->collideTwoParticles(
                  grid[y * cell_count_x + x][p1_i],
                  grid[(y + y_offset) * cell_count_x + x + x_offset][p2_i]);
            }
          }
        }
      }
    }
  }
}

void PhysicSolver::solveParticleCollisionsFixedGridSpatialHash() {
  const float cell_width =
      2 * this->largest_particle_radius; // Cells are squares.
  // WARNING: Grid is padded.
  const uint32_t cell_count_x = std::ceil(this->screen_size.x / cell_width) + 2;
  const uint32_t cell_count_y = std::ceil(this->screen_size.y / cell_width) + 2;
  const uint32_t particle_count = this->particles.size();

  // Stores particle count of each cell in the grid.
  std::vector<uint32_t> grid_cell_count(cell_count_y * cell_count_x, 0);
  // Particles args sorted by cell
  std::vector<uint32_t> particles_sorted(particle_count, 0);

  this->assignParticlesToFixedGridSpatialHash(grid_cell_count, particles_sorted,
                                              cell_count_x, cell_count_y,
                                              cell_width);

  for (uint32_t p_i = 0; p_i < particle_count; p_i++) {
    Particle &p = this->particles[p_i];
    const uint32_t cell_x = (uint32_t)(p.pos.x / cell_width) + 1;
    const uint32_t cell_y = (uint32_t)(p.pos.y / cell_width) + 1;

    const uint32_t cell_x_min = cell_x - 1;
    const uint32_t cell_y_min = cell_y - 1;

    const uint32_t cell_x_max = cell_x + 1;
    const uint32_t cell_y_max = cell_y + 1;

    const uint32_t p_start = grid_cell_count[cell_y * cell_count_x + cell_x];
    const uint32_t p_end = grid_cell_count[cell_y * cell_count_x + cell_x + 1];

    for (uint32_t y = cell_y_min; y <= cell_y_max; y++) {
      for (uint32_t x = cell_x_min; x < cell_x_max; x++) {
        const uint32_t start = grid_cell_count[y * cell_count_x + x];
        const uint32_t end = grid_cell_count[y * cell_count_x + x + 1];
        for (uint32_t i = p_start; i < p_end; i++) {
          for (uint32_t j = start; j < end; j++) {
            collideTwoParticles(particles_sorted[i], particles_sorted[j]);
          }
        }
      }
    }
  }
}

void PhysicSolver::assignParticlesToFixedGrid(
    std::vector<std::vector<uint32_t>> &grid, uint32_t cell_count_x,
    uint32_t cell_count_y, float cell_width) {

  uint32_t particle_count = this->particles.size();

  // Assign particles to grid cells.
  for (uint32_t p_i = 0; p_i < particle_count; p_i++) {
    Particle &p = this->particles[p_i];
    // Assign cell by centroid.
    const uint32_t cell_x = (uint32_t)(p.pos.x / cell_width);
    const uint32_t cell_y = (uint32_t)(p.pos.y / cell_width);

    grid[cell_y * cell_count_x + cell_x].push_back(p_i);

    // Put cells into neighbouring cells if they overflow into them.
    bool in_north = cell_y + 1 < cell_count_y &&
                    p.pos.y + p.radius >= cell_width * (cell_y + 1);
    bool in_east = cell_x + 1 < cell_count_x &&
                   p.pos.x + p.radius >= cell_width * (cell_x + 1);
    bool in_south = cell_y - 1 >= 0 && p.pos.y - p.radius < cell_width * cell_y;
    bool in_west = cell_x - 1 >= 0 && p.pos.x - p.radius < cell_width * cell_x;

    if (in_north) {
      grid[(cell_y + 1) * cell_count_x + cell_x].push_back(p_i);
      if (in_east) {
        grid[(cell_y + 1) * cell_count_x + cell_x + 1].push_back(p_i);
      }
    }
    if (in_east) {
      grid[cell_y * cell_count_x + cell_x + 1].push_back(p_i);
      if (in_south) {
        grid[(cell_y - 1) * cell_count_x + cell_x + 1].push_back(p_i);
      }
    }
    if (in_south) {
      grid[(cell_y - 1) * cell_count_x + cell_x].push_back(p_i);
      if (in_west) {
        grid[(cell_y - 1) * cell_count_x + cell_x - 1].push_back(p_i);
      }
    }
    if (in_west) {
      grid[cell_y * cell_count_x + cell_x - 1].push_back(p_i);
      if (in_north) {
        grid[(cell_y + 1) * cell_count_x + cell_x - 1].push_back(p_i);
      }
    }
  }
}

void PhysicSolver::assignParticlesToFixedGridLoose(
    std::vector<std::vector<uint32_t>> &grid, uint32_t cell_count_x,
    uint32_t cell_count_y, float cell_width) {

  const uint32_t particle_count = this->particles.size();

  // Assign particles to grid cells.
  for (uint32_t p_i = 0; p_i < particle_count; p_i++) {
    Particle &p = this->particles[p_i];
    // Assign cell by centroid. Remember that grid is padded.
    const uint32_t cell_x = (uint32_t)(p.pos.x / cell_width) + 1;
    const uint32_t cell_y = (uint32_t)(p.pos.y / cell_width) + 1;

    grid[cell_y * cell_count_x + cell_x].push_back(p_i);
  }
}

void PhysicSolver::assignParticlesToFixedGridSpatialHash(
    std::vector<uint32_t> &grid_cell_count,
    std::vector<uint32_t> &particles_sorted, uint32_t cell_count_x,
    uint32_t cell_count_y, float cell_width) {

  uint32_t particle_count = this->particles.size();

  // Adjust particle count of respective cells.
  for (uint32_t p_i = 0; p_i < particle_count; p_i++) {
    Particle &p = this->particles[p_i];
    // Assign cell by centroid.
    const uint32_t cell_x = (uint32_t)(p.pos.x / cell_width) + 1;
    const uint32_t cell_y = (uint32_t)(p.pos.y / cell_width) + 1;

    grid_cell_count[cell_y * cell_count_x + cell_x] += 1;
  }

  // Cumulative sum. Remember that last cell is an overflow guard.
  for (uint32_t i = 0; i < grid_cell_count.size() - 1; i++) {
    grid_cell_count[i + 1] += grid_cell_count[i];
  }

  // Populate sorted particles array.
  for (uint32_t p_i = 0; p_i < particle_count; p_i++) {
    Particle &p = this->particles[p_i];
    // Assign cell by centroid.
    const uint32_t cell_x = (uint32_t)(p.pos.x / cell_width) + 1;
    const uint32_t cell_y = (uint32_t)(p.pos.y / cell_width) + 1;

    grid_cell_count[cell_y * cell_count_x + cell_x]--;
    particles_sorted[grid_cell_count[cell_y * cell_count_x + cell_x]] = p_i;
  }
}

void PhysicSolver::collideTwoParticles(const uint32_t p1_i,
                                       const uint32_t p2_i) {
  if (p1_i == p2_i)
    return;

  Particle &p1 = this->particles[p1_i];
  Particle &p2 = this->particles[p2_i];

  const float e = 0.75f;
  const float distanceBetweenCenters = glm::length(p1.pos - p2.pos);
  const float sumOfRadii = p1.radius + p2.radius;

  if (distanceBetweenCenters < sumOfRadii) {
    glm::vec2 collisionAxis = p1.pos - p2.pos;
    glm::vec2 n = collisionAxis / distanceBetweenCenters;
    const float massSum = p1.mass + p2.mass;
    const float massRatio1 = p1.mass / massSum;
    const float massRatio2 = p2.mass / massSum;
    const float delta = e * (sumOfRadii - distanceBetweenCenters);
    p1.pos += massRatio2 * delta * n;
    p2.pos -= massRatio1 * delta * n;
  }
}

// void resolveCollisionsFixedGrid() {
//     const float largestParticleRadius = 7.0f;
//     const float cellWidth = 2 * largestParticleRadius; // Cells are
//     squares. const int cellCountX = std::ceil(m_rectContainer->fillWidth /
//     cellWidth); const int cellCountY =
//     std::ceil(m_rectContainer->fillHeight / cellWidth);
//
//     std::vector<std::vector<std::vector<Particle*>>> grid(cellCountY,
//     std::vector<std::vector<Particle*>>(cellCountX));
//
//     assignParticlesToFixedGrid(grid, cellCountX, cellCountY, cellWidth);
//
//     const int threadCount = 8;
//     std::vector<std::thread> threads(threadCount);
//
//
//     for (int i = 0; i < threadCount; i++) {
//         int xStart = (cellCountX/threadCount)*i;
//         int xEnd = (cellCountX/threadCount)*(i+1)-1;
//         int yStart = 0;
//         int yEnd = cellCountY-1;
//         threads[i] =
//         std::thread(&ParticleSimulation::resolveCollisionsInSubGrid, this,
//         std::ref(grid), xStart, xEnd, yStart, yEnd);
//     }
//
//     for (int i = 0; i < threadCount; i++) {
//         threads[i].join();
//     }
// }

// void
// resolveCollisionsInSubGrid(std::vector<std::vector<std::vector<Particle*>>>&
// grid, int xStart, int xEnd, int yStart, int yEnd) {
//
//     for (int y = yStart; y <= yEnd; y++) {
//         for (int x = xStart; x <= xEnd; x++) {
//             collideParticlesInCell(grid[y][x]);
//         }
//     }
// }

// void
// assignParticlesToFixedGrid(std::vector<std::vector<std::vector<Particle*>>>&
// grid, int cellCountX, int cellCountY, float cellWidth) {
//
//     // Assign particles to grid cells.
//     for (Particle* p : m_particles) {
//         // Assign cell by centroid.
//         const int cellX = (int)(p->pos.x / cellWidth);
//         const int cellY = (int)(p->pos.y / cellWidth);
//         grid[cellY][cellX].push_back(p);
//
//         // Put cells into neighbouring cells if they overflow into them.
//         bool inNorth = cellY+1 < cellCountY && p->pos.y + p->radius >=
//         cellWidth * cellY+1; bool inEast = cellX+1 < cellCountX && p->pos.x
//         + p->radius >= cellWidth * cellX+1; bool inSouth = cellY-1 >= 0 &&
//         p->pos.y - p->radius < cellWidth * cellY; bool inWest = cellX-1 >=
//         0 && p->pos.x - p->radius < cellWidth * cellX;
//
//         if (inNorth) {
//             grid[cellY+1][cellX].push_back(p);
//             if (inEast) {
//                 grid[cellY+1][cellX+1].push_back(p);
//             }
//         }
//         if (inEast) {
//             grid[cellY][cellX+1].push_back(p);
//             if (inSouth) {
//                 grid[cellY-1][cellX+1].push_back(p);
//             }
//         }
//         if (inSouth) {
//             grid[cellY-1][cellX].push_back(p);
//             if (inWest) {
//                 grid[cellY-1][cellX-1].push_back(p);
//             }
//         }
//         if (inWest) {
//             grid[cellY][cellX-1].push_back(p);
//             if (inNorth) {
//                 grid[cellY+1][cellX-1].push_back(p);
//             }
//         }
//     }
//
//
// }
