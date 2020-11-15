#pragma once

#include <functional>

struct UIContext {
  struct {
    int draws;
    bool culling;
    bool wireframe;
    bool passable;
    bool terrain;
    bool static_meshes;
    bool csg;
    bool bounding_boxes;
    bool imported_geodata;
    bool exported_geodata;
  } rendering;

  struct {
    float cell_size;
    float cell_height;
    float walkable_height;
    float wall_angle;
    float walkable_angle;
    float min_walkable_climb;
    float max_walkable_climb;
    std::function<void()> build_handler;
    bool export_;
  } geodata;
};
