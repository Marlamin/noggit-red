// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Action.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/texture_swapper.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/World.h>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

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

      auto brush_widget (new QWidget(this));
      auto brush_layout (new QFormLayout(brush_widget));

      _brush_mode_group = new QGroupBox("Brush mode", brush_widget);
      // _brush_mode_group->setAlignment(Qt::AlignLeft);
      _brush_mode_group->setCheckable(true);
      _brush_mode_group->setChecked(false);

      layout->addRow(_brush_mode_group);
      _brush_mode_group->setLayout(brush_layout);


      _swap_entire_chunk = new QCheckBox(_brush_mode_group);
      _swap_entire_chunk->setText(tr("Entire chunk"));
      _swap_entire_chunk->setCheckState(Qt::CheckState::Unchecked);
      brush_layout->addRow(_swap_entire_chunk);

      _swap_entire_tile = new QCheckBox(_brush_mode_group);
      _swap_entire_tile->setText(tr("Entire tile"));
      _swap_entire_tile->setCheckState(Qt::CheckState::Unchecked);
      brush_layout->addRow(_swap_entire_tile);

      _radius_spin = new QDoubleSpinBox(_brush_mode_group);
      _radius_spin->setRange (0.f, 100.f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);
      brush_layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, _brush_mode_group);
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
            // TODO : action manager
              _world->swapTextureGlobal(_texture_to_swap.value());
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

    std::optional<scoped_blp_texture_reference> const& texture_swapper::texture_to_swap() const
    {
      return _texture_to_swap;
    }

    float texture_swapper::radius() const
    {
      return _radius;
    }

    bool texture_swapper::entireChunk() const
    {
      return _swap_entire_chunk->isChecked();
    }

    bool texture_swapper::entireTile() const
    {
      return _swap_entire_tile->isChecked();
    }

    void texture_swapper::change_radius(float change)
    {
      _radius_spin->setValue(_radius + change);
    }

    bool texture_swapper::brush_mode() const
    {
      return _brush_mode_group->isChecked();
    }

    void texture_swapper::toggle_brush_mode()
    {
      _brush_mode_group->setChecked(!_brush_mode_group->isChecked());
    }

    void texture_swapper::set_texture(std::string const& filename)
    {
      _texture_to_swap = std::move(scoped_blp_texture_reference(filename, _world->getRenderContext()));
    }

    current_texture* const texture_swapper::texture_display()
    {
      return _texture_to_swap_display;
    }
  }
}
