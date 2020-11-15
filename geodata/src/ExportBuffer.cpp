#include "pch.h"

#include "ExportBuffer.h"

namespace geodata {

static constexpr auto MAP_WIDTH_BLOCKS = 256;
static constexpr auto MAP_HEIGHT_BLOCKS = 256;
static constexpr auto MAP_WIDTH_CELLS = 2048;
static constexpr auto MAP_HEIGHT_CELLS = 2048;
static constexpr auto BLOCK_WIDTH_CELLS = 8;
static constexpr auto BLOCK_HEIGHT_CELLS = 8;
static constexpr auto MAX_LAYERS = 128;

ExportBuffer::ExportBuffer(const Geodata &geodata)
    : m_blocks{MAP_WIDTH_BLOCKS * MAP_HEIGHT_BLOCKS},
      m_columns{MAP_WIDTH_CELLS * MAP_HEIGHT_CELLS}, m_cells{MAP_WIDTH_CELLS *
                                                             MAP_HEIGHT_CELLS *
                                                             MAX_LAYERS} {

  for (const auto &cell : geodata.cells) {
    const auto column_index = cell.y + cell.x * MAP_WIDTH_CELLS;
    const auto block_index = cell.y / BLOCK_HEIGHT_CELLS +
                             cell.x / BLOCK_WIDTH_CELLS * MAP_WIDTH_BLOCKS;

    auto &column = m_columns[column_index];
    auto &block = m_blocks[block_index];

    column.layers++;
    block.type = cell.type;

    m_cells[column_index * MAX_LAYERS + column.layers - 1] = &cell;

    ASSERT(column.layers < MAX_LAYERS - 1, "Geodata", // MAX_LAYERS - 1 is ok.
           "Too many layers in column: " << cell.x << " " << cell.y);
  }
}

auto ExportBuffer::block(int x, int y) -> Block & {
  const auto block_index = y + x * MAP_WIDTH_BLOCKS;
  return m_blocks[block_index];
}

auto ExportBuffer::column(int x, int y, int cx, int cy) const
    -> const Column & {
  const auto column_index = (y * BLOCK_HEIGHT_CELLS) + cy +
                            ((x * BLOCK_WIDTH_CELLS) + cx) * MAP_WIDTH_CELLS;
  return m_columns[column_index];
}

auto ExportBuffer::cell(int x, int y, int cx, int cy, int layer) const
    -> const Cell & {

  const auto column_index = (y * BLOCK_HEIGHT_CELLS) + cy +
                            ((x * BLOCK_WIDTH_CELLS) + cx) * MAP_WIDTH_CELLS;
  const auto cell_index = column_index * MAX_LAYERS + layer;
  return *m_cells[cell_index];
}

} // namespace geodata
