// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>
#include <noggit/project/ApplicationProject.h>
#include <QtWidgets/QListWidget>
#include <unordered_set>
#include <string>



class QGridLayout;
class QPushButton;
class QDropEvent;
class QDragEnterEvent;
class QMouseEvent;
class QListWidget;
class QPoint;


namespace Noggit
{
  namespace Ui
  {
    class current_texture;

    class PaletteList : public QListWidget
    {
    public:
      PaletteList(QWidget* parent);

      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      QPoint _start_pos;

    };

    class texture_palette_small : public widget
    {
      Q_OBJECT

    public:
      texture_palette_small (std::shared_ptr<Noggit::Project::NoggitProject> Project, int mapId, QWidget* parent);

      void addTexture();
      void addTextureByFilename(const std::string& filename, bool save_palette = true);

      void removeTexture(QString filename);

      void LoadSavedPalette();
      void SavePalette();

      void removeSelectedTexture();

      void dragEnterEvent(QDragEnterEvent* event) override;
      void dropEvent(QDropEvent* event) override;


    signals:
      void selected(std::string);

    private:

      QGridLayout* layout;

      QListWidget* _texture_list;
      QPushButton* _add_button;
      QPushButton* _remove_button;
      std::unordered_set<std::string> _texture_paths;

      std::shared_ptr<Noggit::Project::NoggitProject> _project;
      int _map_id;
    };
  }
}
