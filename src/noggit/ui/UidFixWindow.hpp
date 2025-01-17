// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <noggit/ui/uid_fix_mode.hpp>
#include <math/trig.hpp>

#include <QtWidgets/QDialog>

class World;

namespace Noggit
{
  namespace Ui
  {
    class UidFixWindow : public QDialog
    {
    Q_OBJECT

    public:
      UidFixWindow (glm::vec3 pos, math::degrees camera_pitch, math::degrees camera_yaw);

    signals:
      void fix_uid  ( glm::vec3 pos
                    , math::degrees camera_pitch
                    , math::degrees camera_yaw
                    , uid_fix_mode uid_fix
                    );
    };
  }
}
