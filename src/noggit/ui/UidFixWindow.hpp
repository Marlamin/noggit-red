// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <math/trig.hpp>

#include <QtWidgets/QDialog>

#include <functional>

class World;

enum class uid_fix_mode
{
  none,
  max_uid,
  fix_all_fail_on_model_loading_error,
  fix_all_fuckporting_edition
};

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
