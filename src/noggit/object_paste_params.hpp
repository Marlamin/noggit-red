// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

namespace Noggit
{
    struct object_paste_params
    {
        float minRotation = -180.f;
        float maxRotation = 180.f;
        float minTilt = -5.f;
        float maxTilt = 5.f;
        float minScale = 0.9f;
        float maxScale = 1.1f;
        bool rotate_on_terrain = true;
    };
}