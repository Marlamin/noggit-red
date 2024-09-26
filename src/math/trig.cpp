// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/trig.hpp>
#include <glm/vec2.hpp>
#include <vector>

bool math::is_inside_of_polygon(const glm::vec2& pos, const std::vector<glm::vec2>& polygon) {
    int n = polygon.size();
    bool inside = false;

    for (int i = 0; i < n; ++i)
    {
        glm::vec2 v1 = polygon[i];
        glm::vec2 v2 = polygon[(i + 1) % n];

        if ((v1.y > pos.y) != (v2.y > pos.y))
        {
            float intersectX = (pos.y - v1.y) * (v2.x - v1.x) / (v2.y - v1.y) + v1.x;
            if (pos.x < intersectX)
            {
                inside = !inside;
            }
        }
    }

    return inside;
}