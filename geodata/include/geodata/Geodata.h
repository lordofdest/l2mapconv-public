#pragma once

#include <vector>

namespace geodata {

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
