// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/TexturePicker.h>

#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/FontNoggit.hpp>
#include <noggit/Selection.h>
#include <noggit/texture_set.hpp>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/World.h>
#include <noggit/tool_enums.hpp>

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>

#include <cassert>

namespace Noggit
{
  namespace Ui
  {
    texture_picker::texture_picker
        (current_texture* current_texture_window, QWidget* parent)
      : widget (parent, Qt::Window)
      , layout (new ::QGridLayout(this))
      , _chunk (nullptr)
      , _main_texture_window(current_texture_window)
    {
      setWindowTitle ("Texture Picker");
      setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);

      for (int i = 0; i < 4; i++)
      {
        current_texture* click_label = new current_texture(false, this);
        connect ( click_label, &ClickableLabel::leftClicked
                , [=]()
                  {
                    if (click_label->_is_swap_selected)
                        return;

                    setTexture(i, current_texture_window);

                    for (unsigned long long i = 0; i < _labels.size(); ++i)
                    {

                        _labels[i]->unselect();

                        if (_labels[i] == click_label && !_labels[i]->_is_swap_selected)
                            _labels[i]->select();
                    }
                  }
                );

        connect(click_label, &ClickableLabel::rightClicked, [=]()
                {
                    if (click_label->_is_selected)
                        return;

                    if (click_label->_is_swap_selected)
                    {
                        click_label->_is_swap_selected = false;
                        click_label->unselectSwap();
                        return;
                    }

                    for (unsigned long long i = 0; i < _labels.size(); ++i)
                    {
                        _labels[i]->unselectSwap();

                        if (_labels[i] == click_label && !_labels[i]->_is_selected)
                        {
                            _labels[i]->selectSwap();
                        }
                    }
                });

        if (click_label->filename() == current_texture_window->filename())
            click_label->select();

        layout->addWidget(click_label, 0, i);
        _labels.push_back(click_label);
      }

      QPushButton* btn_left = new QPushButton (this);
      QPushButton* btn_right = new QPushButton (this);
      btn_left->setIcon(FontAwesomeIcon(FontAwesome::angledoubleleft));
      btn_right->setIcon(FontAwesomeIcon(FontAwesome::angledoubleright));

      btn_left->setMinimumHeight(16);
      btn_right->setMinimumHeight(16);

      auto btn_layout(new QGridLayout);
      btn_layout->addWidget (btn_left, 0, 0);
      btn_layout->addWidget (btn_right, 0, 1);

      layout->addItem(btn_layout, 1, 0, 1, 4, Qt::AlignHCenter | Qt::AlignBottom);

      connect ( btn_left, &QPushButton::clicked
              , [this]
                {
                  emit shift_left();
                }
              );

      connect ( btn_right, &QPushButton::clicked
              , [this]
                {
                  emit shift_right();
                }
              );

      adjustSize();
      setFixedSize(size());

    }

    void texture_picker::updateSelection()
    {
        if (!_chunk)
            return;

        for (size_t index = 0; index < _chunk->texture_set->num(); ++index)
        {
            _labels[index]->unselect();
            if (_main_texture_window->filename() == _labels[index]->filename())
                _labels[index]->select();
        }
    }

    void texture_picker::setMainTexture(current_texture *tex)
    {
        _main_texture_window = tex;
    }

    void texture_picker::getTextures(selection_type lSelection)
    {
      if (lSelection.index() == eEntry_MapChunk)
      {
        MapChunk* chunk = std::get<selected_chunk_type> (lSelection).chunk;
        _chunk = chunk;
        update(false);
      }
    }

    void texture_picker::setTexture
      (size_t id, current_texture* current_texture_window)
    {
      assert(id < _textures.size());

      emit set_texture(_textures[id]);
      current_texture_window->set_texture (_textures[id]->file_key().filepath());
    }

    void texture_picker::shiftSelectedTextureLeft()
    {
      auto&& selectedTexture = selected_texture::get();
      auto ts = _chunk->texture_set.get();
      for (int i = 1; i < ts->num(); i++)
      {
        if (ts->texture(i) == selectedTexture)
        {
          ts->swap_layers(i - 1, i);
          update();
          return;
        }
      }
    }

    void texture_picker::shiftSelectedTextureRight()
    {
      auto&& selectedTexture = selected_texture::get();
      auto ts = _chunk->texture_set.get();
      for (int i = 0; i < ts->num() - 1; i++)
      {
        if (ts->texture(i) == selectedTexture)
        {
          ts->swap_layers(i, i + 1);
          update();
          return;
        }
      }
    }

    void texture_picker::update(bool set_changed)
    {

      if (set_changed)
      {
        _chunk->mt->changed = true;
      }

      _textures.clear();
      size_t index = 0;

      for (; index < _chunk->texture_set->num(); ++index)
      {
        _labels[index]->unselect();
        _textures.push_back(_chunk->texture_set->texture(index));
        _labels[index]->set_texture(_textures[index]->file_key().filepath());
        _labels[index]->show();

        if (_main_texture_window->filename() == _labels[index]->filename())
            _labels[index]->select();
      }

      for (; index < 4U; ++index)
      {
        _labels[index]->hide();
      }
    }
  }
}
