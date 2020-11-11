#include "pch.h"

#include "Postprocessing.h"

namespace geodata {

static constexpr auto RC_FLAT_AREA = 0x1;
static constexpr auto RC_STEEP_AREA = 0x2;

void mark_triangles(float walkableSlope, const float *vertices,
                    const int *triangles, std::size_t triangle_count,
                    unsigned char *areas) {

  const auto steep_slope = std::cos(glm::radians(walkableSlope));

  for (std::size_t i = 0; i < triangle_count; ++i) {
    const auto *triangle = &triangles[i * 3];
    const auto normal =
        glm::triangleNormal(glm::make_vec3(&vertices[triangle[0] * 3]),
                            glm::make_vec3(&vertices[triangle[1] * 3]),
                            glm::make_vec3(&vertices[triangle[2] * 3]));

    if (normal.y < 0.0f) {
      continue;
    }

    if (normal.y <= steep_slope) {
      areas[i] = RC_STEEP_AREA;
    } else {
      areas[i] = RC_FLAT_AREA;
    }
  }
}

void calculate_nswe(const rcHeightfield &hf, int walkable_height,
                    int min_walkable_climb, int max_walkable_climb) {

  const auto max_height = 0xffff;

  for (auto y = 0; y < hf.height; ++y) {
    for (auto x = 0; x < hf.width; ++x) {
      for (auto *span = hf.spans[x + y * hf.width]; span != nullptr;
           span = span->next) {

        if (span->area == RC_NULL_AREA) {
          continue;
        }

        const auto bottom = static_cast<int>(span->smax);
        const auto top = span->next != nullptr
                             ? static_cast<int>(span->next->smin)
                             : max_height;

        span->area &= 0x3;

        for (auto direction = 0; direction < 4; ++direction) {
          const auto dx = x + rcGetDirOffsetX(direction);
          const auto dy = y + rcGetDirOffsetY(direction);

          if (dx < 0 || dy < 0 || dx >= hf.width || dy >= hf.height) {
            span->area |= 1 << (direction + 2);
            continue;
          }

          auto direction_allowed = false;

          for (auto *neighbour = hf.spans[dx + dy * hf.width];
               neighbour != nullptr; neighbour = neighbour->next) {

            if (neighbour->area == RC_NULL_AREA) {
              direction_allowed = false;
              continue;
            }

            const auto neighbour_bottom = static_cast<int>(neighbour->smax);
            const auto neighbour_top =
                neighbour->next != nullptr
                    ? static_cast<int>(neighbour->next->smin)
                    : max_height;

            const auto height = std::min(top, neighbour_top) -
                                std::max(bottom, neighbour_bottom);

            if (height >= walkable_height) {
              const auto climb = neighbour_bottom - bottom;

              if ((span->area & RC_STEEP_AREA) != 0 ||
                  (neighbour->area & RC_STEEP_AREA) != 0) {

                direction_allowed = climb <= min_walkable_climb;
              } else {
                direction_allowed = climb <= max_walkable_climb;
              }

              break;
            }
          }

          if (direction_allowed) {
            span->area |= 1 << (direction + 2);
          }
        }
      }
    }
  }
}

} // namespace geodata
