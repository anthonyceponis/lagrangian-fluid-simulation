#include "spatial_grid.hpp"
#include <algorithm>

#include <iostream>

SpatialGrid::SpatialGrid(std::vector<glm::vec2> &_positions,
                         const float smoothing_radius)
    : spatial_lookup(_positions.size() + 1),
      spatial_indicies(_positions.size()), cell_width(2 * smoothing_radius),
      positions(_positions){};

void SpatialGrid::update() {
  // Reset counts to zero.
  std::fill(this->spatial_lookup.begin(), this->spatial_lookup.end(), 0);

  // Find bucket counts
  for (int32_t i = 0; i < this->positions.size(); i++) {
    glm::ivec2 cell_coord = this->positionToCellCoord(this->positions[i]);
    int32_t cell_hash = this->cellCoordToHash(cell_coord);

    // #Buckets = #Particles with one extra for dealing with overflow
    // Contains start and end indicies for each group.
    this->spatial_lookup[cell_hash]++;
  }

  // Cumulative sum
  for (int32_t i = 1; i < this->spatial_lookup.size(); i++) {
    this->spatial_lookup[i] += this->spatial_lookup[i - 1];
  }

  // Fill spatial indicies
  for (int32_t i = 0; i < this->positions.size(); i++) {
    glm::ivec2 cell_coord = this->positionToCellCoord(this->positions[i]);
    int32_t cell_hash = this->cellCoordToHash(cell_coord);

    this->spatial_lookup[cell_hash]--;
    this->spatial_indicies[this->spatial_lookup[cell_hash]] = i;
  }
}

glm::ivec2 SpatialGrid::positionToCellCoord(glm::vec2 pos) {
  return glm::ivec2(pos / this->cell_width);
}

int32_t SpatialGrid::cellCoordToHash(glm::ivec2 cell_coord) {

  const int32_t prime1 = 15823;
  const int32_t prime2 = 9737333;

  int32_t hash = std::abs((cell_coord.x * prime1) ^ (cell_coord.y * prime2));
  hash %= (this->spatial_lookup.size() - 1);

  return hash;
}
