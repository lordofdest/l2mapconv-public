#include "pch.h"

#include "UISystem.h"

UISystem::UISystem(UIContext &ui_context, WindowContext &window_context,
                   RenderingContext &rendering_context)
    : m_ui_context{ui_context}, m_window_context{window_context},
      m_rendering_context{rendering_context} {

  ASSERT(m_window_context.window_handle != nullptr, "App",
         "Window must be initialized");

  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(m_window_context.window_handle, true);
  ImGui_ImplOpenGL3_Init();

  ImGui::StyleColorsDark();

  // Default rendering settings.
  m_ui_context.rendering.terrain = true;
  m_ui_context.rendering.static_meshes = true;
  m_ui_context.rendering.csg = true;
  m_ui_context.rendering.exported_geodata = true;

  m_ui_context.geodata.export_ = true;

  // Default geodata settings.
  reset_geodata_settings();
}

UISystem::~UISystem() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void UISystem::frame_begin(Timestep frame_time) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  rendering_window(frame_time);
  geodata_window();
}

void UISystem::frame_end(Timestep /*frame_time*/) {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UISystem::rendering_window(Timestep frame_time) const {
  if (m_window_context.keyboard.m) {
    m_ui_context.rendering.wireframe = !m_ui_context.rendering.wireframe;
  }

  const auto &camera_position = m_rendering_context.camera.position();

  ImGui::Begin("Rendering", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("CPU frame time: %f", frame_time.seconds());
  ImGui::Text("Draws: %d", m_ui_context.rendering.draws);
  ImGui::Text("Camera");
  ImGui::Text("\tx: %d", static_cast<int>(camera_position.x));
  ImGui::Text("\ty: %d", static_cast<int>(camera_position.y));
  ImGui::Text("\tz: %d", static_cast<int>(camera_position.z));
  ImGui::Checkbox("Wireframe", &m_ui_context.rendering.wireframe);
  ImGui::Checkbox("Passable", &m_ui_context.rendering.passable);
  ImGui::Checkbox("Terrain", &m_ui_context.rendering.terrain);
  ImGui::Checkbox("Static Meshes", &m_ui_context.rendering.static_meshes);
  ImGui::Checkbox("CSG", &m_ui_context.rendering.csg);
  ImGui::Checkbox("Bounding Boxes", &m_ui_context.rendering.bounding_boxes);
  ImGui::Checkbox("Imported Geodata", &m_ui_context.rendering.imported_geodata);
  ImGui::Checkbox("Exported Geodata", &m_ui_context.rendering.exported_geodata);
  ImGui::End();
}

void UISystem::geodata_window() const {
  ImGui::Begin("Geodata", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::PushItemWidth(50);
  ImGui::InputFloat("Cell Size", &m_ui_context.geodata.cell_size);
  ImGui::InputFloat("Cell Height", &m_ui_context.geodata.cell_height);
  ImGui::InputFloat("Walkable Height", &m_ui_context.geodata.walkable_height);
  ImGui::InputFloat("Walkable Slope", &m_ui_context.geodata.walkable_slope);
  ImGui::InputFloat("Min Walkable Climb",
                    &m_ui_context.geodata.min_walkable_climb);
  ImGui::InputFloat("Max Walkable Climb",
                    &m_ui_context.geodata.max_walkable_climb);

  if (ImGui::Button("Reset")) {
    reset_geodata_settings();
    ASSERT(m_ui_context.geodata.build_handler, "App",
           "Geodata build handler must be defined");
    m_ui_context.geodata.build_handler();
  }

  ImGui::SameLine();

  if (ImGui::Button("Build")) {
    ASSERT(m_ui_context.geodata.build_handler, "App",
           "Geodata build handler must be defined");
    m_ui_context.geodata.build_handler();
  }

  ImGui::SameLine();

  ImGui::Checkbox("Export", &m_ui_context.geodata.export_);

  ImGui::End();
}

void UISystem::reset_geodata_settings() const {
  m_ui_context.geodata.cell_size = 16.0f;
  m_ui_context.geodata.cell_height = 4.0f;
  m_ui_context.geodata.walkable_height = 48.0f;
  m_ui_context.geodata.walkable_slope = 45.0f;
  m_ui_context.geodata.min_walkable_climb = 16.0f;
  m_ui_context.geodata.max_walkable_climb = 24.0f;
}
