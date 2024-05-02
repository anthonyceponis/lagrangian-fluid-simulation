#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

struct SpatialGrid {
  float cell_width;
  // <cell_hash, p_i>
  std::vector<glm::vec2> &positions;
  std::vector<int32_t> spatial_lookup;
  std::vector<int32_t> spatial_indicies;

  SpatialGrid(std::vector<glm::vec2> &_positions, const float smoothing_radius);

  void update();

  glm::ivec2 positionToCellCoord(glm::vec2 pos);

  int32_t cellCoordToHash(glm::ivec2 key);
};
