// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

class QListWidget;

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <string>

enum MinimapGenMode
{
  CURRENT_ADT,
  SELECTED_ADTS,
  MAP
};

struct MinimapRenderSettings
{
  MinimapGenMode export_mode = CURRENT_ADT;
  std::string file_format = ".blp";

  // Render settings
  int resolution = 512;
  bool draw_m2 = false;
  bool draw_wmo = true;
  bool draw_water = true;
  bool draw_adt_grid = false;
  bool draw_elevation = false;
  bool draw_shadows = false;
  bool use_filters = false;
  bool combined_minimap = false;

  // Selection
 // std::array<bool, 4096> selected_tiles = {false};

  std::vector<char> selected_tiles = std::vector<char>( size_t{4096}, false, {} );

  // Filtering
  QListWidget* m2_model_filter_include = nullptr;
  QListWidget* m2_instance_filter_include = nullptr;
  QListWidget* wmo_model_filter_exclude = nullptr;
  QListWidget* wmo_instance_filter_exclude = nullptr;

  // Lighting. Based on default eastern kingdom global light settings (lightparams 12)
  glm::vec3 diffuse_color = {1.0, 0.532352924, 0.0};
  glm::vec3 ambient_color = {0.407770514, 0.508424163, 0.602650642};
  glm::vec4 ocean_color_light = {0.0693173409, 0.294008732, 0.348329663, 0.75};
  glm::vec4 ocean_color_dark = {0.000762581825, 0.113907099, 0.161220074, 1.0};
  glm::vec4 river_color_light = {0.308351517, 0.363725543, 0.0798838138, 0.5};
  glm::vec4 river_color_dark = {0.19945538, 0.320697188, 0.332425594, 1.0};
};