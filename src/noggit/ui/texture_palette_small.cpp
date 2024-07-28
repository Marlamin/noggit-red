// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texture_palette_small.hpp>

#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/project/ApplicationProject.h>

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QApplication>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDrag>
#include <QMimeData>

#include <unordered_set>
#include <string>
#include <algorithm>


namespace Noggit
{
  namespace Ui
  {

    PaletteList::PaletteList(QWidget* parent) : QListWidget(parent)
    {
      setIconSize(QSize(100, 100));
      setViewMode(QListWidget::IconMode);
      setFlow(QListWidget::LeftToRight);
      setWrapping(false);
      setSelectionMode(QAbstractItemView::SingleSelection);
      setAcceptDrops(false);
      setMovement(Movement::Static);
      setResizeMode(QListView::Adjust);
    }

    void PaletteList::mousePressEvent(QMouseEvent* event)
    {
      if (event->button() == Qt::LeftButton)
        _start_pos = event->pos();

      QListWidget::mousePressEvent(event);
    }

    void PaletteList::mouseMoveEvent(QMouseEvent* event)
    {
      QListWidget::mouseMoveEvent(event);

      if (!(event->buttons() & Qt::LeftButton))
        return;
      if ((event->pos() - _start_pos).manhattanLength()
        < QApplication::startDragDistance())
        return;

      const QList<QListWidgetItem*> selected_items = selectedItems();

      for (auto item : selected_items)
      {
        QMimeData* mimeData = new QMimeData;
        mimeData->setText("tileset/" + item->toolTip());


        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(item->icon().pixmap(100, 100));
        drag->exec();
        return;   // we assume only one item can be selected
      }

    }

    texture_palette_small::texture_palette_small (std::shared_ptr<Noggit::Project::NoggitProject> Project, int mapId, QWidget* parent)
      : widget(parent)
      , layout(new ::QGridLayout(this))
      , _project(Project)
      , _map_id(mapId)
    {
      setWindowTitle("Quick Access Texture Palette");
      setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
      setMinimumSize(330, 100);
      setAcceptDrops(true);

      _texture_paths = std::unordered_set<std::string>();
      _texture_list = new PaletteList(this);


      layout->addWidget(_texture_list, 0, 0);

      connect(_texture_list, &QListWidget::itemClicked
        , this
        , [=](QListWidgetItem* item)
        {
          emit selected("tileset/" + item->toolTip().toStdString());
        }
      );

      QVBoxLayout* button_layout = new QVBoxLayout(this);

      _add_button = new QPushButton(this);
      _add_button->setToolTip("Add Selected Texture");
      _add_button->setIcon(FontAwesomeIcon(FontAwesome::plus));
      button_layout->addWidget(_add_button);
      connect(_add_button, &QAbstractButton::clicked, this, &texture_palette_small::addTexture);

      _remove_button = new QPushButton(this);
      _remove_button->setIcon(FontAwesomeIcon(FontAwesome::times));
      button_layout->addWidget(_remove_button);
      connect(_remove_button, &QAbstractButton::clicked, this, &texture_palette_small::removeSelectedTexture);

      button_layout->addStretch();

      layout->addLayout(button_layout, 0, 1);

      LoadSavedPalette();
    }

    void texture_palette_small::LoadSavedPalette()
    {
        auto& saved_palette = _project->TexturePalettes;
        for (auto& palette : saved_palette)
        {
            if (palette.MapId == _map_id)
            {
                for (auto& filename : palette.Filepaths)
                    addTextureByFilename(filename.c_str(), false);
                break;
            }
        }
    }

    void texture_palette_small::SavePalette()
    {
        auto palette_text = Noggit::Project::NoggitProjectTexturePalette();
        palette_text.MapId =_map_id;
        for (auto& path : _texture_paths)
            palette_text.Filepaths.push_back("tileset/" + path);

        _project->saveTexturePalette(palette_text);
    }

    void texture_palette_small::addTexture()
    {

      std::string filename;
      if (Noggit::Ui::selected_texture::get())
        filename = Noggit::Ui::selected_texture::get().value()->file_key().filepath();
      else
        filename = "tileset\\generic\\black.blp";

      addTextureByFilename(filename);

    }

    void texture_palette_small::addTextureByFilename(const std::string& filename, bool save_palette)
    {

      QString display_name = QString(filename.c_str()).remove("tileset/");
      display_name.remove("tileset\\");

      for (auto path : _texture_paths)
        if (path == display_name.toStdString())
          return;

      _texture_paths.emplace(display_name.toStdString());

      QListWidgetItem* list_item = new QListWidgetItem(_texture_list);
      list_item->setIcon(*BLPRenderer::getInstance().render_blp_to_pixmap(filename, _texture_list->iconSize().width(), _texture_list->iconSize().height()));
      list_item->setToolTip(display_name);
      list_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

      _texture_list->addItem(list_item);

      // auto saving whenever an object is added, could change it to manually save
      if (save_palette)
          SavePalette();
    }

    void texture_palette_small::removeTexture(QString filename)
    {

      filename.remove("tileset/");
      QList<QListWidgetItem*> tilesets = _texture_list->findItems(filename, Qt::MatchExactly);

      for (auto tileset : tilesets)
        if (tileset->toolTip() == filename)
        {
          _texture_paths.erase(filename.toStdString());
          _texture_list->removeItemWidget(tileset);
          _add_button->setDisabled(false);
          delete tileset;
          return;
        }


    }

    void texture_palette_small::removeSelectedTexture()
    {

      QList<QListWidgetItem*> selected_items = _texture_list->selectedItems();

      for (auto item : selected_items)
      {

        for (auto path : _texture_paths)
          if (path == item->toolTip().toStdString())
          {
            _texture_paths.erase(path);
            _texture_list->removeItemWidget(item);
            _add_button->setDisabled(false);
            delete item;
            return;
          }

      }
    }

    void texture_palette_small::dragEnterEvent(QDragEnterEvent* event)
    {
      if (event->mimeData()->hasText()
        && (_texture_paths.find(event->mimeData()->text().remove("tileset/").toStdString()) == _texture_paths.end())
        )
          event->accept();
    }

    void texture_palette_small::dropEvent(QDropEvent* event)
    {

      addTextureByFilename(event->mimeData()->text().toStdString());
      event->accept();
    }

  }
}
