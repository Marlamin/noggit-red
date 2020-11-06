#include <noggit/ui/main_window.hpp>

#include <noggit/DBC.h>
#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/World.h>
#include <noggit/ui/About.h>
#include <noggit/MapView.h>
#include <noggit/ui/SettingsPanel.h>
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/uid_storage.hpp>
#include <noggit/Red/MapCreationWizard/Ui/MapCreationWizard.hpp>
#include <noggit/ui/font_awesome.hpp>
#include <noggit/ui/FramelessWindow.hpp>

#include <QtGui/QCloseEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>

#include <boost/format.hpp>

#ifdef USE_MYSQL_UID_STORAGE
  #include <mysql/mysql.h>

  #include <QtCore/QSettings>
#endif

#include "revision.h"

#include "ui_TitleBar.h"
#include <external/framelesshelper/framelesswindowsmanager.h>


namespace noggit
{
  namespace ui
  {
    main_window::main_window()
      : QMainWindow (nullptr)
      , _null_widget (new QWidget (this))
    {

      std::stringstream title;
      title << "Noggit - " << STRPRODUCTVER;
      setWindowTitle (QString::fromStdString (title.str()));
      setWindowIcon (QIcon (":/icon"));

      setCentralWidget (_null_widget);

      createBookmarkList();

      _about = new about(this);

      _menuBar = menuBar();

      _settings = new settings(this);

      QSettings settings;

      if (!settings.value("systemWindowFrame", false).toBool())
      {
        QWidget *widget = new QWidget(this);
        Ui::TitleBar* titleBarWidget = setupFramelessWindow(widget, this, minimumSize(), maximumSize(), true);
        titleBarWidget->horizontalLayout->insertWidget(2, _menuBar);
        setMenuWidget(widget);
      }

      _menuBar->setNativeMenuBar(settings.value("nativeMenubar", true).toBool());

      auto file_menu (_menuBar->addMenu ("&Noggit"));

      auto settings_action (file_menu->addAction ("Settings"));
      QObject::connect ( settings_action, &QAction::triggered
                       , [&]
                         {
                           _settings->show();
                         }
                       );

      auto about_action (file_menu->addAction ("About"));
      QObject::connect ( about_action, &QAction::triggered
                       , [&]
                         {
                           _about->show();
                         }
                       );

      auto mapmenu_action (file_menu->addAction ("Exit"));
      QObject::connect ( mapmenu_action, &QAction::triggered
                       , [this]
                         {
                            close();
                         }
                       );


      _menuBar->adjustSize();

      build_menu();
    }

    void main_window::check_uid_then_enter_map
                        ( math::vector_3d pos
                        , math::degrees camera_pitch
                        , math::degrees camera_yaw
                        , bool from_bookmark
                        )
    {
      QSettings settings;
#ifdef USE_MYSQL_UID_STORAGE
      bool use_mysql = settings.value("project/mysql/enabled", false).toBool();

      if ((use_myqsl && mysql::hasMaxUIDStoredDB(_world->getMapID()))
        || uid_storage::hasMaxUIDStored(_world->getMapID())
         )
      {
        _world->mapIndex.loadMaxUID();
        enterMapAt(pos, camera_pitch, camera_yaw, from_bookmark);
      }
#else
      if (uid_storage::hasMaxUIDStored(_world->getMapID()))
      {
        if (settings.value("uid_startup_check", true).toBool())
        {
          enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::max_uid, from_bookmark);
        }
        else
        {
          _world->mapIndex.loadMaxUID();
          enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::none, from_bookmark);
        }
      }
#endif
      else
      {
        auto uidFixWindow(new uid_fix_window(pos, camera_pitch, camera_yaw));
        uidFixWindow->show();

        connect( uidFixWindow
               , &noggit::ui::uid_fix_window::fix_uid
               , [this, from_bookmark] 
                   ( math::vector_3d pos
                   , math::degrees camera_pitch
                   , math::degrees camera_yaw
                   , uid_fix_mode uid_fix
                   )
                 {
                   enterMapAt(pos, camera_pitch, camera_yaw, uid_fix, from_bookmark);
                 }
               );
      }
    }

    void main_window::enterMapAt ( math::vector_3d pos
                                 , math::degrees camera_pitch
                                 , math::degrees camera_yaw
                                 , uid_fix_mode uid_fix
                                 , bool from_bookmark
                                 )
    {
      _map_creation_wizard->destroyFakeWorld();

      _stack_widget->removeWidget(_map_view);
      _map_view =  (new MapView (camera_yaw, camera_pitch, pos, this, std::move (_world), uid_fix, from_bookmark));
      connect(_map_view, &MapView::uid_fix_failed, [this]() { prompt_uid_fix_failure(); });

      _stack_widget->addWidget(_map_view);
      _stack_widget->setCurrentIndex(1);

      map_loaded = true;

    }

    void main_window::loadMap(int mapID)
    {
      _minimap->world (nullptr);

      _world.reset();

      for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
      {
        if (it->getInt(MapDB::MapID) == mapID)
        {
          _world = std::make_unique<World> (it->getString(MapDB::InternalName), mapID);
          _minimap->world (_world.get());
          emit map_selected(mapID);

          return;
        }
      }

      LogError << "Map with ID " << mapID << " not found. Failed loading." << std::endl;
    }

    void main_window::build_menu()
    {
      _stack_widget = new QStackedWidget(this);

      auto widget (new QWidget(_stack_widget));
      _stack_widget->addWidget(widget);

      auto layout (new QHBoxLayout (widget));
      layout->setAlignment(Qt::AlignLeft);

      QListWidget* bookmarks_table (new QListWidget (widget));
      _continents_table = new QListWidget (widget);
      _dungeons_table = new QListWidget (widget);
      _raids_table = new QListWidget (widget);
      _battlegrounds_table = new QListWidget (widget);
      _arenas_table = new QListWidget (widget);

      std::array<QListWidget*, 5> type_to_table
          {{_continents_table, _dungeons_table, _raids_table, _battlegrounds_table, _arenas_table}};

      for (auto& table : type_to_table)
      {
        QObject::connect ( table, &QListWidget::itemClicked
            , [this] (QListWidgetItem* item)
                           {
                             loadMap (item->data (Qt::UserRole).toInt());
                           }
        );
      }


      QTabWidget* entry_points_tabs (new QTabWidget (widget));
      entry_points_tabs->setMaximumWidth(600);

      entry_points_tabs->addTab (_continents_table, "Continents");
      entry_points_tabs->addTab (_dungeons_table, "Dungeons");
      entry_points_tabs->addTab (_raids_table, "Raids");
      entry_points_tabs->addTab (_battlegrounds_table, "Battlegrounds");
      entry_points_tabs->addTab (_arenas_table, "Arenas");
      entry_points_tabs->addTab (bookmarks_table, "Bookmarks");

      layout->addWidget (entry_points_tabs);

      build_map_lists();

      qulonglong bookmark_index (0);
      for (auto entry : mBookmarks)
      {
        auto item (new QListWidgetItem (entry.name.c_str(), bookmarks_table));
        item->setData (Qt::UserRole, QVariant (bookmark_index++));
      }

      QObject::connect ( bookmarks_table, &QListWidget::itemDoubleClicked
                       , [this] (QListWidgetItem* item)
                         {
                           auto& entry (mBookmarks.at (item->data (Qt::UserRole).toInt()));

                           _world.reset();

                           for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
                           {
                             if (it->getInt(MapDB::MapID) == entry.mapID)
                             {
                               _world = std::make_unique<World> (it->getString(MapDB::InternalName), entry.mapID);
                               check_uid_then_enter_map ( entry.pos
                                                        , math::degrees (entry.camera_pitch)
                                                        , math::degrees (entry.camera_yaw)
                                                        , true
                                                        );

                               return;
                             }
                           }
                         }
                       );


      _minimap = new minimap_widget (this);
      _minimap->draw_boundaries (true);

      QObject::connect
        ( _minimap,  &minimap_widget::map_clicked
        , [this] (::math::vector_3d const& pos)
          {
            check_uid_then_enter_map(pos, math::degrees(30.f), math::degrees(90.f));
          }
        );

      auto right_side = new QTabWidget(this);

      auto minimap_holder = new QWidget(this);
      minimap_holder->setContentsMargins(0, 0, 0, 0);
      auto minimap_holder_layout = new QHBoxLayout(this);
      minimap_holder->setLayout(minimap_holder_layout);
      minimap_holder_layout->addWidget(_minimap);
      minimap_holder_layout->setAlignment(Qt::AlignCenter);
      right_side->addTab(minimap_holder, "Enter map");
      minimap_holder->setAccessibleName("main_menu_minimap_holder");

      _map_creation_wizard = new noggit::Red::MapCreationWizard::Ui::MapCreationWizard(this);

      _map_wizard_connection = connect(_map_creation_wizard, &noggit::Red::MapCreationWizard::Ui::MapCreationWizard::map_dbc_updated
          ,[=]
          {
            build_map_lists();
          }
      );

      right_side->addTab(_map_creation_wizard, "Edit map");

      layout->addWidget(right_side);

      setCentralWidget (_stack_widget);

      _minimap->adjustSize();
    }

    void noggit::ui::main_window::build_map_lists()
    {

      std::array<QListWidget*, 5> type_to_table
          {{_continents_table, _dungeons_table, _raids_table, _battlegrounds_table, _arenas_table}};

      for (auto& table : type_to_table)
      {
        table->clear();
      }

      for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
      {
        MapEntry e;
        e.mapID = i->getInt(MapDB::MapID);
        e.name = i->getLocalizedString(MapDB::Name);
        e.areaType = i->getUInt(MapDB::AreaType);

        if (e.areaType < 0 || e.areaType > 4 || !World::IsEditableWorld(e.mapID))
          continue;

        auto item (new QListWidgetItem (QString::number(e.mapID) + " - " + QString::fromUtf8 (e.name.c_str()), type_to_table[e.areaType]));
        item->setData (Qt::UserRole, QVariant (e.mapID));
      }
    }


    void main_window::createBookmarkList()
    {
      mBookmarks.clear();

      std::ifstream f("bookmarks.txt");
      if (!f.is_open())
      {
        LogDebug << "No bookmarks file." << std::endl;
        return;
      }

      std::string basename;
      std::size_t areaID;
      BookmarkEntry b;
      int mapID = -1;
      while (f >> mapID >> b.pos.x >> b.pos.y >> b.pos.z >> b.camera_yaw >> b.camera_pitch >> areaID)
      {
        if (mapID == -1)
        {
          continue;
        }

        std::stringstream temp;
        temp << MapDB::getMapName(mapID) << ": " << AreaDB::getAreaName(areaID);
        b.name = temp.str();
        b.mapID = mapID;
        mBookmarks.push_back(b);
      }
      f.close();
    }

    void main_window::closeEvent (QCloseEvent* event)
    {
      if (map_loaded)
      {
        event->ignore();
        prompt_exit(event);
      }
      else
      {
        event->accept();
      }
    }

    void main_window::prompt_exit(QCloseEvent* event)
    {
      emit exit_prompt_opened();

      QMessageBox prompt;
      prompt.setIcon (QMessageBox::Warning);
      prompt.setWindowFlags(Qt::WindowStaysOnTopHint);
      prompt.setText ("Exit?");
      prompt.setInformativeText ("Any unsaved changes will be lost.");
      prompt.addButton ("Exit", QMessageBox::DestructiveRole);
      prompt.addButton ("Return to menu", QMessageBox::AcceptRole);
      prompt.setDefaultButton (prompt.addButton ("Cancel", QMessageBox::RejectRole));
      prompt.setWindowFlags (Qt::CustomizeWindowHint | Qt::WindowTitleHint);

      prompt.exec();

      switch (prompt.buttonRole(prompt.clickedButton()))
      {
        case QMessageBox::AcceptRole:
          _stack_widget->setCurrentIndex(0);
          _stack_widget->removeWidget(_map_view);
          delete _map_view;
          _minimap->world (nullptr);

          map_loaded = false;
          break;
        case QMessageBox::DestructiveRole:
          setCentralWidget(_null_widget = new QWidget(this));
          event->accept();
          break;
        default:
          event->ignore();
          break;
      }
    }

    void main_window::prompt_uid_fix_failure()
    {
      _stack_widget->setCurrentIndex(0);

      QMessageBox::critical
        ( nullptr
        , "UID fix failed"
        , "The UID fix couldn't be done because some models were missing or fucked up.\n"
          "The models are listed in the log file."
        , QMessageBox::Ok
        );
    }
  }
}
