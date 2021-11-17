#ifndef NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_TOOL_HPP
#define NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_TOOL_HPP

#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QSlider>
#include <QDial>
#include <QDoubleSpinBox>
#include <glm/vec3.hpp>

class QPixmap;
class World;

namespace math
{
  struct vector_3d;
}

namespace noggit
{
  struct bool_toggle_property;

  namespace Red::StampMode::Ui
  {
    class Tool : public QWidget
    {
      Q_OBJECT
      public:
        explicit
        Tool(bool_toggle_property* showPalette, float* cursorRotation, QWidget* parent = nullptr);
        auto getOuterRadius(void) const -> float;
        auto getInnerRadius(void) const -> float;
        auto stamp(World* world, glm::vec3 const& pos, float dt, bool doAdd) const -> void;
        public slots:
        void setPixmap(QPixmap const* pixmap);
      private:
        float* _cursorRotation;
        float _radiusOuter;
        float _radiusInner;
        float _rotation;
        QFormLayout _layout;
        QLabel _label;
        QPushButton _btnPalette;
        QPixmap const* _curPixmap;
        QSlider _sliderRadiusOuter;
        QDoubleSpinBox _spinboxRadiusOuter;
        QSlider _sliderRadiusInner;
        QDoubleSpinBox _spinboxRadiusInner;
        QDial _dialRotation;
    };
  }
}

#endif//NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_TOOL_HPP
