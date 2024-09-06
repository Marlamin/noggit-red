// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "tool_enums.hpp"
#include "MinimapRenderSettings.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct MinimapRenderSettings;

namespace Noggit
{
    struct ToolDrawParameters
    {
        float radius = 0.0f;
        float inner_radius = 0.0f;
        float angle = 0.0f;
        float orientation = 0.0f;
        glm::vec3 ref_pos;
        bool angled_mode = false;
        bool use_ref_pos = false;
        bool show_unpaintable_chunks = false;
        CursorType cursor_type = CursorType::CIRCLE;
        eTerrainType terrain_type = eTerrainType::eTerrainType_Flat;
        int displayed_water_layer = -1;
        glm::vec4 cursor_color = { 1.f, 1.f, 1.f, 1.f };
        MinimapRenderSettings minimapRenderSettings;
    };
}