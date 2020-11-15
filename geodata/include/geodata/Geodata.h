#pragma once

#include <vector>

namespace geodata {

enum Direction {
  DIRECTION_N = 0x8,
  DIRECTION_S = 0x4,
  DIRECTION_W = 0x2,
  DIRECTION_E = 0x1,
};

enum BlockType {
  BLOCK_SIMPLE,
  BLOCK_COMPLEX,
  BLOCK_MULTILAYER,
};

struct Cell {
  int x : 16;
  int y : 16;
  int z : 16;
  BlockType type : 4;
  bool north : 1;
  bool south : 1;
  bool west : 1;
  bool east : 1;
};

struct Geodata {
  std::vector<Cell> cells;
};

} // namespace geodata
