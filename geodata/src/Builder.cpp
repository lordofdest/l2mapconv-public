#include "pch.h"

#include "Postprocessing.h"

#include <geodata/Builder.h>

namespace geodata {

auto Builder::build(const Map &map, const BuilderSettings &settings) const
    -> Geodata {

  // Configure.
  const auto cell_size = settings.cell_size;
  const auto cell_height = settings.cell_height;
  const auto walkable_height =
      static_cast<int>(std::ceil(settings.walkable_height / cell_height));
  const auto walkable_slope = settings.walkable_slope;
  const auto min_walkable_climb =
      static_cast<int>(std::floor(settings.min_walkable_climb / cell_height));
  const auto max_walkable_climb =
      static_cast<int>(std::floor(settings.max_walkable_climb / cell_height));

  // Flip bounding box for Recast (Y <-> Z).
  const auto *source_bb_min = glm::value_ptr(map.bounding_box().min());
  const auto *source_bb_max = glm::value_ptr(map.bounding_box().max());
  float bb_min[3] = {source_bb_min[0], source_bb_min[2], source_bb_min[1]};
  float bb_max[3] = {source_bb_max[0], source_bb_max[2], source_bb_max[1]};

  // Calculate grid size.
  auto width = 0;
  auto height = 0;
  rcCalcGridSize(static_cast<float *>(bb_min), static_cast<float *>(bb_max),
                 cell_size, &width, &height);

  rcContext context{};

  // Create heightfield.
  auto *hf = rcAllocHeightfield();
  rcCreateHeightfield(&context, *hf, width, height,
                      static_cast<float *>(bb_min),
                      static_cast<float *>(bb_max), cell_size, cell_height);

  // Prepare geometry data.
  const auto *vertices = glm::value_ptr(map.vertices().front());
  const auto vertex_count = map.vertices().size();
  const auto *triangles = reinterpret_cast<const int *>(map.indices().data());
  const auto triangle_count = map.indices().size() / 3;

  // Rasterize triangles.
  auto areas = std::make_unique<unsigned char[]>(triangle_count);
  mark_triangles(walkable_slope, vertices, triangles, triangle_count,
                 areas.get());
  rcRasterizeTriangles(&context, vertices, vertex_count, triangles, areas.get(),
                       triangle_count, *hf, min_walkable_climb);

  // Filter low height spans.
  rcFilterWalkableLowHeightSpans(&context, walkable_height, *hf);

  // Calculate NSWE.
  calculate_nswe(*hf, walkable_height, min_walkable_climb, max_walkable_climb);

  // Convert heightfield to geodata.
  Geodata geodata;

  const auto depth = static_cast<int>((bb_max[2] - bb_min[2]) / cell_height);
  const auto z_offset = 7; // TODO: Calculate by cell_height.

  for (auto y = 0; y < height; ++y) {
    for (auto x = 0; x < width; ++x) {
      for (auto *span = hf->spans[x + y * width]; span != nullptr;
           span = span->next) {

        const auto area = span->area & 0x3;
        const auto nswe = span->area >> 2;

        if (area == RC_NULL_AREA || nswe == 0) {
          continue;
        }

        geodata.cells.push_back({
            x,
            y,
            static_cast<int>((span->smax - depth / 2 + z_offset) * cell_height),
            BLOCK_MULTILAYER,
            (nswe & 0x8) != 0,
            (nswe & 0x2) != 0,
            (nswe & 0x1) != 0,
            (nswe & 0x4) != 0,
        });
      }
    }
  }

  // Cleanup.
  rcFreeHeightField(hf);

  return geodata;
}

} // namespace geodata
