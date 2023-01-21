// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texture_swapper.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <noggit/tool_enums.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace Noggit
{
  namespace Ui
  {
    texture_swapper::texture_swapper ( QWidget* parent
                                     , const glm::vec3* camera_pos
                                     , MapView* map_view
                                     )
      : QWidget (parent)
      , _texture_to_swap()
      , _radius(15.f)
      , _world(map_view->getWorld())
    {
      setWindowTitle ("Swap");
      setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);

      auto layout (new QFormLayout (this));

      _texture_to_swap_display = new current_texture(true, this);

      QPushButton* select = new QPushButton("Select", this);
      QPushButton* swap_adt = new QPushButton("Swap ADT", this);
      QPushButton* swap_global = new QPushButton("Swap Global(All ADTs)", this);
      QPushButton* remove_text_adt = new QPushButton(tr("Remove this texture from ADT"), this);

      layout->addRow(new QLabel("Texture to swap"));
      layout->addRow(_texture_to_swap_display);
      layout->addRow(select);
      layout->addRow(swap_adt);
      layout->addRow(swap_global);
      layout->addRow(remove_text_adt);

      _brush_mode_group = new QGroupBox("Brush mode", this);
      _brush_mode_group->setCheckable(true);
      _brush_mode_group->setChecked(false);
      layout->addRow(_brush_mode_group);

      auto brush_content (new QWidget(_brush_mode_group));
      auto brush_layout (new QFormLayout(brush_content));
      _brush_mode_group->setLayout(brush_layout);

      _swap_entire_chunk = new QCheckBox(brush_content);
      _swap_entire_chunk->setText(tr("Entire chunk"));
      _swap_entire_chunk->setCheckState(Qt::CheckState::Unchecked);
      brush_layout->addRow(_swap_entire_chunk);

      _radius_spin = new QDoubleSpinBox(brush_content);
      _radius_spin->setRange (0.f, 100.f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);
      brush_layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, brush_content);
      _radius_slider->setRange (0, 100);
      _radius_slider->setSliderPosition (_radius);
      brush_layout->addRow (_radius_slider);      
      
      connect(select, &QPushButton::clicked, [&, map_view]() {

        map_view->context()->makeCurrent(map_view->context()->surface());
        OpenGL::context::scoped_setter const _ (::gl, map_view->context());
        _texture_to_swap = selected_texture::get();
        if (_texture_to_swap)
        {
          _texture_to_swap_display->set_texture(_texture_to_swap.value()->file_key().filepath());
        }
      });

      connect(swap_adt, &QPushButton::clicked, [this, camera_pos, map_view]() {
        if (_texture_to_swap)
        {
          ActionManager::instance()->beginAction(map_view, ActionFlags::eCHUNKS_TEXTURE);
          _world->swapTexture (*camera_pos, _texture_to_swap.value());
          ActionManager::instance()->endAction();
        }
      });

      connect(swap_global, &QPushButton::clicked, [this, camera_pos, map_view]() {
          if (_texture_to_swap)
          {
              // ActionManager::instance()->beginAction(map_view, ActionFlags::eCHUNKS_TEXTURE);
              _world->swapTextureGlobal(_texture_to_swap.value());
              // ActionManager::instance()->endAction();
          }
          });

      connect(remove_text_adt, &QPushButton::clicked, [this, camera_pos, map_view]() {
          if (_texture_to_swap)
          {
              ActionManager::instance()->beginAction(map_view, ActionFlags::eCHUNKS_TEXTURE);
              _world->removeTexture(*camera_pos, _texture_to_swap.value());
              ActionManager::instance()->endAction();
          }
          });

      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&](double v)
                {
                  QSignalBlocker const blocker (_radius_slider);
                  _radius = v;
                  _radius_slider->setSliderPosition ((int)std::round (v));

                }
              );

      connect ( _radius_slider, &QSlider::valueChanged
              , [&](int v)
                {
                  QSignalBlocker const blocker (_radius_spin);
                  _radius = v;
                  _radius_spin->setValue(v);
                }
              );
    }

    void texture_swapper::change_radius(float change)
    {
      _radius_spin->setValue(_radius + change);
    }

    void texture_swapper::set_texture(std::string const& filename)
    {
      _texture_to_swap = std::move(scoped_blp_texture_reference(filename, _world->getRenderContext()));
    }
  }
}
