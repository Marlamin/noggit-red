#include <noggit/ui/windows/about/About.h>
#include <noggit/DBC.h>
#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/World.h>
#include <noggit/ContextObject.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include <noggit/MapView.h>
#include <noggit/ui/windows/settingsPanel/SettingsPanel.h>
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/UidFixWindow.hpp>
#include <noggit/uid_storage.hpp>
#include <noggit/ui/tools/MapCreationWizard/Ui/MapCreationWizard.hpp>
#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/FramelessWindow.hpp>
#include <noggit/ui/tools/UiCommon/StackedWidget.hpp>
#include <noggit/project/ApplicationProject.h>
#include <BlizzardDatabase.h>
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
#include <QtWidgets/QComboBox>
#include <noggit/ui/windows/noggitWindow/widgets/MapListItem.hpp>
#include <noggit/ui/windows/noggitWindow/widgets/MapBookmarkListItem.hpp>
#include <QtNetwork/QTcpSocket>
#include <sstream>
#include <QSysInfo>
#include <QStandardPaths>
#include <QDir>
#include <QIcon>
#include <noggit/ui/windows/noggitWindow/components/BuildMapListComponent.hpp>
#include <noggit/application/Utils.hpp>

#ifdef USE_MYSQL_UID_STORAGE
#include <mysql/mysql.h>

#include <QtCore/QSettings>
#endif

#include "revision.h"

#include "ui_TitleBar.h"
#include <external/framelesshelper/framelesswindowsmanager.h>
#include <noggit/ui/tools/ViewportManager/ViewportManager.hpp>

namespace Noggit::Ui::Windows
{
  NoggitWindow::NoggitWindow(std::shared_ptr<Noggit::Application::NoggitApplicationConfiguration> application,
                             std::shared_ptr<Noggit::Project::NoggitProject> project)
      : QMainWindow(nullptr)
      , _null_widget(new QWidget(this))
      , _applicationConfiguration(application)
      , _project(project)
  {

    std::stringstream title;
    title << "Noggit - " << STRPRODUCTVER;
    setWindowTitle(QString::fromStdString(title.str()));
    setWindowIcon(QIcon(":/icon"));

    Log << "Project version : " << Noggit::Project::ClientVersionFactory::MapToStringVersion(project->projectVersion).c_str() << std::endl;

    if (project->projectVersion == Project::ProjectVersion::WOTLK)
    {
      OpenDBs(project->ClientData);
    }

    setCentralWidget(_null_widget);

    // The default value is AnimatedDocks | AllowTabbedDocks.
    setDockOptions(AnimatedDocks | AllowNestedDocks | AllowTabbedDocks | GroupedDragging);

    _about = new about(this);
    _settings = new settings(this);

    _menuBar = menuBar();

    QSettings settings;

    if (!settings.value("systemWindowFrame", true).toBool())
    {
      QWidget* widget = new QWidget(this);
      ::Ui::TitleBar* titleBarWidget = setupFramelessWindow(widget, this, minimumSize(), maximumSize(), true);
      titleBarWidget->horizontalLayout->insertWidget(2, _menuBar);
      setMenuWidget(widget);
    }

    _menuBar->setNativeMenuBar(settings.value("nativeMenubar", true).toBool());

    auto file_menu(_menuBar->addMenu("&Noggit"));

    auto settings_action(file_menu->addAction("Settings"));
    QObject::connect(settings_action, &QAction::triggered, [&]
                     {
                       _settings->show();
                     }
    );

    auto about_action(file_menu->addAction("About"));
    QObject::connect(about_action, &QAction::triggered, [&]
                     {
                       _about->show();
                     }
    );

    auto mapmenu_action(file_menu->addAction("Exit"));
    QObject::connect(mapmenu_action, &QAction::triggered, [this]
                     {
                       close();
                     }
    );

    auto proj_selec_action(file_menu->addAction("Exit to Project Selection"));
    QObject::connect(proj_selec_action, &QAction::triggered, [this]
        {
            auto noggit = Noggit::Application::NoggitApplication::instance();
            auto project_selection = new Noggit::Ui::Windows::NoggitProjectSelectionWindow(noggit);
            project_selection->show();
            close();
        }
    );

    _menuBar->adjustSize();

    _buildMapListComponent = std::make_unique<Component::BuildMapListComponent>();

    buildMenu();
  }

  void NoggitWindow::check_uid_then_enter_map
      (glm::vec3 pos, math::degrees camera_pitch, math::degrees camera_yaw, bool from_bookmark
      )
  {
    QSettings settings;
#ifdef USE_MYSQL_UID_STORAGE
    bool use_mysql = settings.value("project/mysql/enabled", false).toBool();

    bool valid_conn = false;
    if (use_mysql)
    {
        valid_conn = mysql::testConnection(true);
    }

    if ((valid_conn && mysql::hasMaxUIDStoredDB(_world->getMapID()))
      || uid_storage::hasMaxUIDStored(_world->getMapID())
       )
    {

      _world->mapIndex.loadMaxUID();
      enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::none, from_bookmark);
    }
#else
    if (uid_storage::hasMaxUIDStored(_world->getMapID()))
    {
      if (settings.value("uid_startup_check", true).toBool())
      {
        enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::max_uid, from_bookmark);
      } else
      {
        _world->mapIndex.loadMaxUID();
        enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::none, from_bookmark);
      }
    }
#endif
    else
    {
      auto uid_fix_window(new UidFixWindow(pos, camera_pitch, camera_yaw));
      uid_fix_window->show();

      connect(uid_fix_window, &Noggit::Ui::UidFixWindow::fix_uid, [this, from_bookmark]
                  (glm::vec3 pos, math::degrees camera_pitch, math::degrees camera_yaw, uid_fix_mode uid_fix
                  )
              {
                enterMapAt(pos, camera_pitch, camera_yaw, uid_fix, from_bookmark);
              }
      );
    }
  }

  void
  NoggitWindow::enterMapAt(glm::vec3 pos, math::degrees camera_pitch, math::degrees camera_yaw, uid_fix_mode uid_fix,
                           bool from_bookmark
  )
  {
      if (_world->mapIndex.hasAGlobalWMO())
      {
          // enter at mdoel's position
          // pos = glm::vec3(_world->mWmoEntry[0], _world->mWmoEntry.pos[1], _world->mWmoEntry.pos[2]);

          // better, enter at model's max extent, facing toward min extent
          auto min_extent = glm::vec3(_world->mWmoEntry.extents[0][0], _world->mWmoEntry.extents[0][1], _world->mWmoEntry.extents[0][2]);
          auto max_extent = glm::vec3(_world->mWmoEntry.extents[1][0], _world->mWmoEntry.extents[1][1] * 2, _world->mWmoEntry.extents[1][2]);
          float dx = min_extent.x - max_extent.x;
          float dy = min_extent.z - max_extent.z; // flipping z and y works better for some reason
          float dz = min_extent.y - max_extent.y;

          pos = max_extent;

          camera_yaw = math::degrees(math::radians(std::atan2(dx, dy)));

          float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
          camera_pitch = math::degrees(math::radians(std::asin(dz / distance)));

      }


    _map_creation_wizard->destroyFakeWorld();
    _map_view = (new MapView(camera_yaw, camera_pitch, pos, this, _project, std::move(_world), uid_fix, from_bookmark));
    connect(_map_view, &MapView::uid_fix_failed, [this]()
    { promptUidFixFailure(); });
    connect(_settings, &settings::saved, [this]()
    { if (_map_view) _map_view->onSettingsSave(); });

    _stack_widget->addWidget(_map_view);
    _stack_widget->setCurrentIndex(1);

    map_loaded = true;

  }

  void NoggitWindow::applyFilterSearch(const QString &name, int type, int expansion, bool wmo_maps)
  {
      for (int i = 0; i < _continents_table->count(); ++i)
      {
          auto item_widget = _continents_table->item(i);
          auto widget = qobject_cast<Noggit::Ui::Widget::MapListItem*>(_continents_table->itemWidget(item_widget));

          if (!widget)
              continue;

          item_widget->setHidden(false);

          if (!widget->name().contains(name, Qt::CaseInsensitive))
          {
              item_widget->setHidden(true);
              continue;
          }

          if (!(widget->type() == (type - 2)) && type != 0)
          {
              item_widget->setHidden(true);
              continue;
          }

          if (!(widget->expansion() == (expansion - 1)) && expansion != 0)
          {
              item_widget->setHidden(true);
          }

          if (!(widget->wmo_map() == wmo_maps))
          {
              item_widget->setHidden(true);
          }
      }
  }

  void NoggitWindow::loadMap(int map_id)
  {
    _minimap->world(nullptr);

    _world.reset();

    auto table = _project->ClientDatabase->LoadTable("Map", readFileAsIMemStream);
    auto record = table.Record(map_id);

    _world = std::make_unique<World>(record.Columns["Directory"].Value, map_id, Noggit::NoggitRenderContext::MAP_VIEW);
    _minimap->world(_world.get());

    _project->ClientDatabase->UnloadTable("Map");

    emit mapSelected(map_id);

  }

  void NoggitWindow::buildMenu()
  {
    _stack_widget = new StackedWidget(this);
    _stack_widget->setAutoResize(true);

    setCentralWidget(_stack_widget);

    auto widget(new QWidget(_stack_widget));
    _stack_widget->addWidget(widget);

    auto layout(new QHBoxLayout(widget));
    layout->setAlignment(Qt::AlignLeft);
    QListWidget* bookmarks_table(new QListWidget(widget));
    _continents_table = new QListWidget(widget);
    QObject::connect(_continents_table, &QListWidget::itemClicked, [this](QListWidgetItem* item)
                     {
                       loadMap(item->data(Qt::UserRole).toInt());
                     }
    );


    QTabWidget* entry_points_tabs(new QTabWidget(widget));
    //entry_points_tabs->addTab(_continents_table, "Maps");

     auto add_btn = new QPushButton("Add New Map", this);
     add_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
     add_btn->setAccessibleName("map_wizard_add_button");

    /* set-up widget for seaching etc... through _continents_table */
    {
        QWidget* _first_tab = new QWidget(this);
        QVBoxLayout* _first_tab_layout = new QVBoxLayout();
        _first_tab->setLayout(_first_tab_layout);

        QGroupBox* _group_search = new QGroupBox(tr("Search"), this);

        QLineEdit* _line_edit_search = new QLineEdit(this);
        QComboBox* _combo_search = new QComboBox(this);
        _combo_search->addItems(QStringList() <<
                                tr("All") <<
                                tr("Unknown") <<
                                tr("Continent") <<
                                tr("Dungeon") <<
                                tr("Raid") <<
                                tr("Battleground") <<
                                tr("Arena") <<
                                tr("Scenario"));
        _combo_search->setCurrentIndex(0);

        QComboBox* _combo_exp_search = new QComboBox(this);
        _combo_exp_search->addItem(tr("All"));
        _combo_exp_search->addItem(QIcon(":/icon-classic"), tr("Classic"));
        _combo_exp_search->addItem(QIcon(":/icon-burning"), tr("Burning Cursade"));
        _combo_exp_search->addItem(QIcon(":/icon-wrath"), tr("Wrath of the Lich King"));
        _combo_exp_search->addItem(QIcon(":/icon-cata"), tr("Cataclism"));
        _combo_exp_search->addItem(QIcon(":/icon-panda"), tr("Mist of Pandaria"));
        _combo_exp_search->addItem(QIcon(":/icon-warlords"), tr("Warlords of Draenor"));
        _combo_exp_search->addItem(QIcon(":/icon-legion"), tr("Legion"));
        _combo_exp_search->addItem(QIcon(":/icon-battle"), tr("Battle for Azeroth"));
        _combo_exp_search->addItem(QIcon(":/icon-shadow"), tr("Shadowlands"));
        _combo_exp_search->setCurrentIndex(0);

        QCheckBox* _wmo_maps_search = new QCheckBox("Display WMO maps (No terrain)", this);

        QObject::connect(_line_edit_search, QOverload<const QString&>::of(&QLineEdit::textChanged), [this, _combo_search, _combo_exp_search, _wmo_maps_search](const QString &name)
                         {
                             applyFilterSearch(name, _combo_search->currentIndex(), _combo_exp_search->currentIndex(), _wmo_maps_search->isChecked());
                         });

        QObject::connect(_combo_search, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, _line_edit_search, _combo_exp_search, _wmo_maps_search](int index)
                         {
                             applyFilterSearch(_line_edit_search->text(), index, _combo_exp_search->currentIndex(), _wmo_maps_search->isChecked());
                         });

        QObject::connect(_combo_exp_search, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, _line_edit_search, _combo_search, _wmo_maps_search](int index)
                         {
                             applyFilterSearch(_line_edit_search->text(), _combo_search->currentIndex(), index, _wmo_maps_search->isChecked());
                         });

        QObject::connect(_wmo_maps_search, &QCheckBox::stateChanged, [this, _line_edit_search, _combo_search, _combo_exp_search](bool b)
                         {
                             applyFilterSearch(_line_edit_search->text(), _combo_search->currentIndex(), _combo_exp_search->currentIndex(), b);
                         });

        QFormLayout* _group_layout = new QFormLayout();
        _group_layout->addRow(tr("Name : "), _line_edit_search);
        _group_layout->addRow(tr("Type : "), _combo_search);
        _group_layout->addRow(tr("Expansion : "), _combo_exp_search);
        _group_layout->addRow( _wmo_maps_search);
        _group_search->setLayout(_group_layout);

        _first_tab_layout->addWidget(_group_search);
        _first_tab_layout->addSpacing(5);
        _first_tab_layout->addWidget(_continents_table);
        _first_tab_layout->addWidget(add_btn);

        entry_points_tabs->addTab(_first_tab, tr("Maps"));
    }

    entry_points_tabs->addTab(bookmarks_table, "Bookmarks");
    entry_points_tabs->setFixedWidth(310);
    layout->addWidget(entry_points_tabs);

    _buildMapListComponent->buildMapList(this);

    qulonglong bookmark_index(0);
    for (auto entry: _project->Bookmarks)
    {
      auto item = new QListWidgetItem(bookmarks_table);

      auto bookmark_data = Widget::MapListBookmarkData();
      bookmark_data.MapName = QString::fromStdString(entry.name);
      bookmark_data.Position = entry.position;

      auto map_bookmark_item = new Widget::MapListBookmarkItem(bookmark_data, bookmarks_table);

      item->setData(Qt::UserRole, QVariant(bookmark_index++));
      item->setSizeHint(map_bookmark_item->minimumSizeHint());
      bookmarks_table->setItemWidget(item, map_bookmark_item);
    }

    QObject::connect(bookmarks_table, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item)
                     {

                       auto& entry(_project->Bookmarks.at(item->data(Qt::UserRole).toInt()));

                       _world.reset();

                       for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
                       {
                         if (it->getInt(MapDB::MapID) == entry.map_id)
                         {
                           _world = std::make_unique<World>(it->getString(MapDB::InternalName),
                                                            entry.map_id, Noggit::NoggitRenderContext::MAP_VIEW);
                           check_uid_then_enter_map(entry.position, math::degrees(entry.camera_pitch), math::degrees(entry.camera_yaw),
                                                    true
                           );
                           return;
                         }
                       }
                     }
    );


    _minimap = new minimap_widget(this);
    _minimap->draw_boundaries(true);
    //_minimap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QObject::connect(_minimap, &minimap_widget::map_clicked, [this](::glm::vec3 const& pos)
                     {
                        if (_world->mapIndex.hasAGlobalWMO()) // skip uid check
                            enterMapAt(pos, math::degrees(30.f), math::degrees(90.f), uid_fix_mode::none, false);
                        else
                            check_uid_then_enter_map(pos, math::degrees(30.f), math::degrees(90.f));
                     }
    );

    _right_side = new QTabWidget(this);

    auto minimap_holder = new QScrollArea(this);
    minimap_holder->setWidgetResizable(true);
    minimap_holder->setAlignment(Qt::AlignCenter);
    minimap_holder->setWidget(_minimap);

    _right_side->addTab(minimap_holder, "Enter map");
    minimap_holder->setAccessibleName("main_menu_minimap_holder");

    _map_creation_wizard = new Noggit::Ui::Tools::MapCreationWizard::Ui::MapCreationWizard(_project, this);

    _map_wizard_connection = connect(_map_creation_wizard,
                                     &Noggit::Ui::Tools::MapCreationWizard::Ui::MapCreationWizard::map_dbc_updated, [=]
                                     {
                                       _buildMapListComponent->buildMapList(this);
                                     }
    );

    _right_side->addTab(_map_creation_wizard, "Edit map");

    layout->addWidget(_right_side);

    connect(add_btn, &QPushButton::clicked
        , [&]()
        {
            _right_side->setCurrentIndex(1);
            _map_creation_wizard->addNewMap();
        });

    //setCentralWidget (_stack_widget);

    _minimap->adjustSize();
  }

  void NoggitWindow::closeEvent(QCloseEvent* event)
  {
    if (map_loaded)
    {
      event->ignore();
      promptExit(event);
    } else
    {
      event->accept();
    }
  }

  void NoggitWindow::handleEventMapListContextMenuPinMap(int mapId, std::string MapName)
  {
    _project->pinMap(mapId, MapName);
    _buildMapListComponent->buildMapList(this);
  }

  void NoggitWindow::handleEventMapListContextMenuUnpinMap(int mapId)
  {
    _project->unpinMap(mapId);
    _buildMapListComponent->buildMapList(this);
  }

  void NoggitWindow::promptExit(QCloseEvent* event)
  {
    emit exitPromptOpened();

    QMessageBox prompt;
    prompt.setModal(true);
    prompt.setIcon(QMessageBox::Warning);
    prompt.setText("Exit?");
    prompt.setInformativeText("Any unsaved changes will be lost.");
    prompt.addButton("Exit", QMessageBox::DestructiveRole);
    prompt.addButton("Return to menu", QMessageBox::AcceptRole);
    prompt.setDefaultButton(prompt.addButton("Cancel", QMessageBox::RejectRole));
    prompt.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);

    prompt.exec();

    switch (prompt.buttonRole(prompt.clickedButton()))
    {
      case QMessageBox::AcceptRole:
        _stack_widget->setCurrentIndex(0);
        _stack_widget->removeLast();
        delete _map_view;
        _map_view = nullptr;
        _minimap->world(nullptr);

        map_loaded = false;
        break;
      case QMessageBox::DestructiveRole:
        Noggit::Ui::Tools::ViewportManager::ViewportManager::unloadAll();
        setCentralWidget(_null_widget = new QWidget(this));
        event->accept();
        break;
      default:
        event->ignore();
        break;
    }
  }

  void NoggitWindow::promptUidFixFailure()
  {
    _stack_widget->setCurrentIndex(0);

    QMessageBox::critical
        (nullptr, "UID fix failed", "The UID fix couldn't be done because some models were missing or fucked up.\n"
                                    "The models are listed in the log file.", QMessageBox::Ok
        );
  }
}
