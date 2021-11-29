// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "object_palette.hpp"

#include <noggit/ui/font_awesome.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/MapView.h>

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


namespace noggit
{
    namespace ui
    {

        ObjectList::ObjectList(QWidget* parent) : QListWidget(parent)
        {
          setIconSize(QSize(100, 100));
          setViewMode(QListWidget::IconMode);
          setFlow(QListWidget::LeftToRight);
          setWrapping(true);
          setSelectionMode(QAbstractItemView::SingleSelection);
          setAcceptDrops(false);
          setMovement(Movement::Static);
        }

        void ObjectList::mousePressEvent(QMouseEvent* event)
        {
          if (event->button() == Qt::LeftButton)
            _start_pos = event->pos();

          QListWidget::mousePressEvent(event);
        }

        void ObjectList::mouseMoveEvent(QMouseEvent* event)
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
            mimeData->setText(item->toolTip());


            QDrag* drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setPixmap(item->icon().pixmap(100, 100));
            drag->exec();
            return;   // we assume only one item can be selected
          }

        }

        ObjectPalette::ObjectPalette (MapView* map_view, QWidget* parent)
            : widget(parent)
            , layout(new ::QGridLayout(this))
            , _map_view(map_view)
        {
          setWindowTitle("Object Palette");
          setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
          setMinimumSize(330, 100);
          setAcceptDrops(true);

          _object_paths = std::unordered_set<std::string>();
          _object_list = new ObjectList(this);


          layout->addWidget(_object_list, 0, 0);

          _preview_renderer = new noggit::ui::tools::PreviewRenderer(_object_list->iconSize().width(), _object_list->iconSize().height(),
                                                  noggit::NoggitRenderContext::OBJECT_PALETTE_PREVIEW, this);
          _preview_renderer->setVisible(false);

          // just to initialize context, ugly-ish
          _preview_renderer->setModelOffscreen("world/wmo/azeroth/buildings/human_farm/farm.wmo");
          _preview_renderer->renderToPixmap();

          connect(_object_list, &QListWidget::itemClicked
              , this
              , [=](QListWidgetItem* item)
                  {
                      _map_view->getObjectEditor()->copy(item->toolTip().toStdString());
                  }
          );


          QVBoxLayout* button_layout = new QVBoxLayout(this);

          _add_button = new QPushButton(this);
          _add_button->setIcon(font_awesome_icon(font_awesome::plus));
          button_layout->addWidget(_add_button);
          connect(_add_button, &QAbstractButton::clicked, this, &ObjectPalette::addObject);

          _remove_button = new QPushButton(this);
          _remove_button->setIcon(font_awesome_icon(font_awesome::times));
          button_layout->addWidget(_remove_button);
          connect(_remove_button, &QAbstractButton::clicked, this, &ObjectPalette::removeSelectedTexture);

          button_layout->addStretch();

          layout->addLayout(button_layout, 0, 1);


        }

        void ObjectPalette::addObject()
        {

          std::string const& display_name = reinterpret_cast<noggit::ui::tools::AssetBrowser::Ui::AssetBrowserWidget*>(
              _map_view->getAssetBrowser()->widget())->getFilename();

          addObjectByFilename(display_name.c_str());
        }


        void ObjectPalette::removeObject(QString filename)
        {

          QList<QListWidgetItem*> objects = _object_list->findItems(filename, Qt::MatchExactly);

          for (auto obj : objects)
            if (obj->toolTip() == filename)
            {
              _object_paths.erase(filename.toStdString());
              _object_list->removeItemWidget(obj);
              _add_button->setDisabled(false);
              delete obj;
              return;
            }


        }

        void ObjectPalette::removeSelectedTexture()
        {

          QList<QListWidgetItem*> selected_items = _object_list->selectedItems();

          for (auto item : selected_items)
          {

            for (auto path : _object_paths)
              if (path == item->toolTip().toStdString())
              {
                _object_paths.erase(path);
                _object_list->removeItemWidget(item);
                _add_button->setDisabled(false);
                delete item;
                return;
              }

          }
        }

        void ObjectPalette::dragEnterEvent(QDragEnterEvent* event)
        {
          if (event->mimeData()->hasText()
              && (_object_paths.find(event->mimeData()->text().toStdString()) == _object_paths.end())
              )
            event->accept();
        }

        void ObjectPalette::addObjectByFilename(QString const& filename)
        {
          if (filename.isEmpty())
            return;

          for (auto path : _object_paths)
            if (path == filename.toStdString())
              return;

          _object_paths.emplace(filename.toStdString());

          QListWidgetItem* list_item = new QListWidgetItem(_object_list);
          _preview_renderer->setModelOffscreen(filename.toStdString());
          list_item->setIcon(*_preview_renderer->renderToPixmap());
          list_item->setData(Qt::DisplayRole, filename);
          list_item->setToolTip(filename);
          list_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
          list_item->setText("");

          _object_list->addItem(list_item);

        }

        void ObjectPalette::dropEvent(QDropEvent* event)
        {
          addObjectByFilename(event->mimeData()->text());
          event->accept();
        }

        ObjectPalette::~ObjectPalette()
        {
          delete _preview_renderer;
        }

    }
}
