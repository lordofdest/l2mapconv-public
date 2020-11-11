#include "pch.h"

#include "L2JSerializer.h"

namespace geodata {

static constexpr auto map_width_blocks = 256;
static constexpr auto map_height_blocks = 256;
static constexpr auto map_width_cells = 2048;
static constexpr auto map_height_cells = 2048;
static constexpr auto block_width_cells = 8;
static constexpr auto block_height_cells = 8;
static constexpr auto max_layers = 256;
static constexpr auto cell_height = 8;

auto L2JSerializer::deserialize(std::istream &input) const -> Geodata {
  Geodata geodata;

  for (auto x = 0; x < map_width_blocks; ++x) {
    for (auto y = 0; y < map_height_blocks; ++y) {
      std::uint8_t type = 0;
      input >> utils::extract<llvm::ulittle8_t>(type);

      if (type == BLOCK_SIMPLE) {
        std::int16_t z = 0;
        input >> utils::extract<llvm::little16_t>(z);

        geodata.cells.push_back({
            static_cast<std::int16_t>(x * block_width_cells),
            static_cast<std::int16_t>(y * block_height_cells),
            z,
            static_cast<BlockType>(type),
            true,
            true,
            true,
            true,
        });
      } else if (type == BLOCK_COMPLEX) {
        for (auto cx = 0; cx < block_width_cells; ++cx) {
          for (auto cy = 0; cy < block_height_cells; ++cy) {
            geodata.cells.push_back(
                read_complex_block_cell(input, type, x, y, cx, cy));
          }
        }
      } else if (type == BLOCK_MULTILAYER) {
        for (auto cx = 0; cx < block_width_cells; ++cx) {
          for (auto cy = 0; cy < block_height_cells; ++cy) {
            std::uint8_t layers = 0;
            input >> utils::extract<llvm::ulittle8_t>(layers);

            for (auto i = 0; i < layers; ++i) {
              geodata.cells.push_back(
                  read_complex_block_cell(input, type, x, y, cx, cy));
            }
          }
        }
      }
    }
  }

  return geodata;
}

void L2JSerializer::serialize(const Geodata &geodata,
                              std::ostream &output) const {

  struct Block {
    std::uint8_t type;
  };

  struct Column {
    std::uint8_t layers;
  };

  const auto blocks =
      std::make_unique<Block[]>(map_width_blocks * map_height_blocks);
  const auto columns =
      std::make_unique<Column[]>(map_width_cells * map_height_cells);
  const auto cells = std::make_unique<const Cell *[]>(
      map_width_cells * map_height_cells * max_layers);

  // Fill geodata buffers.
  for (const auto &cell : geodata.cells) {
    const auto column_index = cell.y + cell.x * map_width_cells;
    const auto block_index = cell.y / block_height_cells +
                             cell.x / block_width_cells * map_width_blocks;

    auto &column = columns[column_index];
    auto &block = blocks[block_index];

    column.layers++;
    block.type = cell.type;

    // TODO: Columns memory layout can be optimized.
    cells[column_index * max_layers + column.layers - 1] = &cell;

    ASSERT(column.layers < max_layers - 1, "Geodata", // max_layers - 1 is ok.
           "Too many layers in column: " << cell.x << " " << cell.y);
  }

  // Write geodata.
  for (auto x = 0; x < map_width_blocks; ++x) {
    for (auto y = 0; y < map_height_blocks; ++y) {
      const auto block_index = y + x * map_width_blocks;
      const auto &block = blocks[block_index];

      output.put(block.type);

      // TODO: Add simple and complex blocks support.
      if (block.type == BLOCK_MULTILAYER) {
        for (auto cx = 0; cx < block_width_cells; ++cx) {
          for (auto cy = 0; cy < block_height_cells; ++cy) {
            const auto column_index =
                (y * block_width_cells) + cy +
                ((x * block_height_cells) + cx) * map_height_cells;
            const auto &column = columns[column_index];

            output.put(column.layers);

            for (auto layer = 0; layer < column.layers; ++layer) {
              const auto cell_index = column_index * max_layers + layer;
              const auto &cell = cells[cell_index];

              // Calculate NSWE.
              const std::uint8_t nswe =
                  (cell->north ? 0x8 : 0) | (cell->south ? 0x4 : 0) |
                  (cell->west ? 0x2 : 0) | (cell->east ? 0x1 : 0);

              std::int16_t z = cell->z;

              // Round cell height.
              if (z % cell_height != 0) {
                z = (z / cell_height - 1) * cell_height;
              }

              z = (z << 1) | nswe;             // Add NSWE.
              z = (z & 0x00ff) | (z & 0xff00); // Swap bytes.

              output.write(reinterpret_cast<char *>(&z), sizeof(z));
            }
          }
        }
      }
    }
  }
}

auto L2JSerializer::read_complex_block_cell(std::istream &input,
                                            std::uint8_t type, int x, int y,
                                            int cx, int cy) const -> Cell {

  std::int16_t z = 0;
  input >> utils::extract<llvm::little16_t>(z);

  const std::uint8_t nswe = z & 0x000f;
  z = z & 0xfff0;
  z = z >> 1;

  return {
      static_cast<std::int16_t>(x * block_width_cells + cx),
      static_cast<std::int16_t>(y * block_width_cells + cy),
      z,
      static_cast<BlockType>(type),
      (nswe & 0x8) != 0,
      (nswe & 0x4) != 0,
      (nswe & 0x2) != 0,
      (nswe & 0x1) != 0,
  };
}

} // namespace geodata
