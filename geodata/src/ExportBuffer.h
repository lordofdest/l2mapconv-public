#pragma once

#include <geodata/Geodata.h>

#include <vector>

namespace geodata {

class ExportBuffer {
public:
  struct Block {
    std::uint8_t type;
    std::int16_t z; // TODO: Workaround for simple blocks.
  };

  struct Column {
    std::uint8_t layers;
  };

  explicit ExportBuffer(const Geodata &geodata);

  auto block(int x, int y) -> Block &;
  auto column(int x, int y, int cx, int cy) const -> const Column &;
  auto cell(int x, int y, int cx = 0, int cy = 0, int layer = 0) const
      -> const Cell &;

private:
  std::vector<Block> m_blocks;
  std::vector<Column> m_columns;
  std::vector<const Cell *> m_cells;
};

} // namespace geodata
