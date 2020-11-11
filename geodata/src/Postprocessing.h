#pragma once

#include "Recast.h"

namespace geodata {

void mark_triangles(float walkableSlope, const float *vertices,
                    const int *triangles, std::size_t triangle_count,
                    unsigned char *areas);

void calculate_nswe(const rcHeightfield &hf, int walkable_height,
                    int min_walkable_climb, int max_walkable_climb);

} // namespace geodata
