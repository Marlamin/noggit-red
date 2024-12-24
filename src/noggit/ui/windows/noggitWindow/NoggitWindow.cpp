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
#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/FramelessWindow.hpp>
#include <noggit/ui/tools/UiCommon/StackedWidget.hpp>
#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/noggitWindow/widgets/MapListItem.hpp>
#include <noggit/ui/windows/noggitWindow/widgets/MapBookmarkListItem.hpp>
#include <noggit/ui/windows/noggitWindow/components/BuildMapListComponent.hpp>
#include <noggit/application/Utils.hpp>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/ui/tools/MapCreationWizard/Ui/MapCreationWizard.hpp>
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
#include <QtNetwork/QTcpSocket>
#include <QSysInfo>
#include <QStandardPaths>
#include <QDir>
#include <QIcon>
#include <QScrollArea>

#include <sstream>
#include <chrono>

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
    else
    {
        LogError << "NoggitWindow() : Unsupported project version, skipping loading DBCs." << std::endl;
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

    auto proj_selec_action(file_menu->addAction("Exit to Project Selection"));
    QObject::connect(proj_selec_action, &QAction::triggered, [this]
        {
            // auto noggit = Noggit::Application::NoggitApplication::instance();
            // auto project_selection = new Noggit::Ui::Windows::NoggitProjectSelectionWindow(noggit);
            // project_selection->show();

            exit_to_project_selection = true;
            close();
        }
    );

    auto mapmenu_action(file_menu->addAction("Exit"));
    QObject::connect(mapmenu_action, &QAction::triggered, [this]
                     {
                       close();
                     }
    );

    _menuBar->adjustSize();

    _buildMapListComponent = std::make_unique<Component::BuildMapListComponent>();

    _map_creation_wizard = new Noggit::Ui::Tools::MapCreationWizard::Ui::MapCreationWizard(_project, this);

    buildMenu();
  }

  void NoggitWindow::check_uid_then_enter_map
      (glm::vec3 pos, math::degrees camera_pitch, math::degrees camera_yaw, bool from_bookmark
      )
  {
    QSettings settings;
    assert(getWorld());

    unsigned int world_map_id = getWorld()->getMapID();

#ifdef USE_MYSQL_UID_STORAGE
    bool use_mysql = settings.value("project/mysql/enabled", false).toBool();

    bool valid_conn = false;
    if (use_mysql)
    {
        valid_conn = mysql::testConnection(true);
    }

    if ((valid_conn && mysql::hasMaxUIDStoredDB(world_map_id))
      || uid_storage::hasMaxUIDStored(world_map_id)
       )
    {

      getWorld()->mapIndex.loadMaxUID();
      enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::none, from_bookmark);
    }
#else
    if (uid_storage::hasMaxUIDStored(world_map_id))
    {
      if (settings.value("uid_startup_check", true).toBool())
      {
        enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::max_uid, from_bookmark);
      } else
      {
        getWorld()->mapIndex.loadMaxUID();
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
    World* world = getWorld();

    if (world->mapIndex.hasAGlobalWMO())
    {
        // enter at mdoel's position
        // pos = glm::vec3(_world->mWmoEntry[0], _world->mWmoEntry.pos[1], _world->mWmoEntry.pos[2]);

        // better, enter at model's max extent, facing toward min extent
        auto min_extent = glm::vec3(world->mWmoEntry.extents[0][0], world->mWmoEntry.extents[0][1], world->mWmoEntry.extents[0][2]);
        auto max_extent = glm::vec3(world->mWmoEntry.extents[1][0], world->mWmoEntry.extents[1][1] * 2, world->mWmoEntry.extents[1][2]);
        float dx = min_extent.x - max_extent.x;
        float dy = min_extent.z - max_extent.z; // flipping z and y works better for some reason
        float dz = min_extent.y - max_extent.y;

        pos = { world->mWmoEntry.pos[0], world->mWmoEntry.pos[1], world->mWmoEntry.pos[2] };

        camera_yaw = math::degrees(math::radians(std::atan2(dx, dy)));

        float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        camera_pitch = -math::degrees(math::radians(std::asin(dz / distance)));

    }


    // _map_creation_wizard->destroyFakeWorld();
    _map_view = (new MapView(camera_yaw, camera_pitch, pos, this, _project, std::move(_map_creation_wizard->_world), uid_fix, from_bookmark));
    connect(_map_view, &MapView::uid_fix_failed, [this]()
    { promptUidFixFailure(); });
    connect(_settings, &settings::saved, [this]()
    { if (_map_view) _map_view->onSettingsSave(); });

    // _app_toolbar = new QToolBar("Application", this);
    // _app_toolbar->setOrientation(Qt::Horizontal);
    // addToolBar(_app_toolbar);

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
    // _minimap->world(nullptr);

    // World is now created only here in
    // void MapCreationWizard::selectMap(int map_id)
    emit mapSelected(map_id);

    /*
    _world.reset();

    auto table = _project->ClientDatabase->LoadTable("Map", readFileAsIMemStream);
    auto record = table.Record(map_id);

    _world = std::make_unique<World>(record.Columns["Directory"].Value, map_id, Noggit::NoggitRenderContext::MAP_VIEW);
    */

    _minimap->world(getWorld());

    _project->ClientDatabase->UnloadTable("Map");
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
    _continents_table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    _continents_table->setSelectionBehavior(QAbstractItemView::SelectItems);

    // in some situations like when returning to menu an item is selected and itemSelectionChanged won't fire when reclicking it
    // so might need to also connect itemClicked but then they both trigger at the same time.
    QObject::connect(_continents_table, &QListWidget::itemSelectionChanged, [this]()
      {
        QSignalBlocker const blocker(_continents_table);
        QListWidgetItem* const item = _continents_table->currentItem();
        if (item)
        {
          loadMap(item->data(Qt::UserRole).toInt());
        }
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
        QSettings settings;
        _combo_search->setCurrentIndex(settings.value("mapComboSearch", 2).toInt());

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
                             QSettings settings;
                             settings.setValue("mapComboSearch", index);
                             settings.sync();
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

        entry_points_tabs->addTab(bookmarks_table, "Bookmarks");
        entry_points_tabs->setFixedWidth(310);
        layout->addWidget(entry_points_tabs);

        _buildMapListComponent->buildMapList(this);
        applyFilterSearch(_line_edit_search->text(), _combo_search->currentIndex(), _combo_exp_search->currentIndex(), _wmo_maps_search->isChecked());
    }

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

                       _map_creation_wizard->_world.reset();

                       for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
                       {
                         if (it->getInt(MapDB::MapID) == entry.map_id)
                         {
                           //     emit mapSelected(map_id); to update UI
                           _map_creation_wizard->_world = std::make_unique<World>(it->getString(MapDB::InternalName),
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
                        if (getWorld()->mapIndex.hasAGlobalWMO()) // skip uid check
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

    _map_wizard_connection = connect(_map_creation_wizard,
                                     &Noggit::Ui::Tools::MapCreationWizard::Ui::MapCreationWizard::map_dbc_updated, 
                                [this](int map_id = -1)
                                     {
                                       _buildMapListComponent->buildMapList(this);

                                       // if a new map was added select it
                                       if (map_id >= 0)
                                       {
                                           for (int i = 0; i < _continents_table->count(); ++i)
                                           {
                                               QListWidgetItem* item = _continents_table->item(i);
                                               if (item && item->data(Qt::UserRole).toInt() == map_id)
                                               {
                                                   _continents_table->setCurrentItem(item); // calls itemSelectionChanged -> loadmap
                                                   _continents_table->scrollToItem(item);
                                                   _right_side->setCurrentIndex(0); // swap to "enter map" tab
                                                   // loadMap(map_id);
                                                   break;
                                               }
                                           }
                                       }
                                     });

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
      if (exit_to_project_selection)
      {
        auto noggit = Noggit::Application::NoggitApplication::instance();
        auto project_selection = new Noggit::Ui::Windows::NoggitProjectSelectionWindow(noggit);
        project_selection->show();
      }
      else
        event->accept();
    }
    exit_to_project_selection = false;
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

  World* NoggitWindow::getWorld()
  {
    return _map_creation_wizard->getWorld();
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
    if (!exit_to_project_selection)
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
        if (exit_to_project_selection)
        {
            auto noggit = Noggit::Application::NoggitApplication::instance();
            auto project_selection = new Noggit::Ui::Windows::NoggitProjectSelectionWindow(noggit);
            project_selection->show();
        }
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
  void NoggitWindow::startWowClient()
  {
      // TODO auto login

      // TODO attach to process to allow memory edits

      // std::filesystem::path WoW_path = std::filesystem::path(Noggit::Project::CurrentProject::get()->ClientPath) / "Wow.exe";
      std::filesystem::path WoW_path = std::filesystem::path(_project->ClientPath) / "Wow.exe";

      QString program_path = WoW_path.string().c_str();
      QFileInfo checkFile(program_path);

      QStringList arguments;
      // arguments << "-console"; // deosn't seem to work
      arguments << "-windowed";
      // auto login using https://github.com/FrostAtom/awesome_wotlk, requires patched Wow.exe
      // arguments << "-login \"LOGIN\" -password \"PASSWORD\" -realmlist \"REALMLIST\" -realmname \"REALMNAME\"";


      if (checkFile.exists() && checkFile.isFile())
      {
          QProcess* process = new QProcess(this); // this parent?
          process->start(program_path, arguments);

          //Noggit::Application::NoggitApplication::instance().
          if (!process->waitForStarted()) {
              // qWarning("Failed to start process");
              QMessageBox::information(this, "Error", "Failed to start process Wow.exe");
          }
      }
      else
      {
          // qWarning("File does not exist");
          QMessageBox::information(this, "Error", std::format("File {} does not exist", WoW_path.string()).c_str());
      }
  }

  void NoggitWindow::patchWowClient()
  {
      // Option to make folder patch ?

      QDialog* mpq_patch_params = new QDialog(this);
      mpq_patch_params->setWindowFlags(Qt::Dialog);
      mpq_patch_params->setWindowTitle("Patch Export Settings");
      QVBoxLayout* mpq_patch_params_layout = new QVBoxLayout(mpq_patch_params);

      mpq_patch_params_layout->addWidget(new QLabel("<font color=orange><h4> Warning :\
          \nErrors can cause MPQ Corruption, make sure to have backup.</h4></font>\
          \nWhile Noggit is writting the archive, files are not accessible by the client, please wait.", mpq_patch_params));

      mpq_patch_params_layout->addWidget(new QLabel("MPQ Name:", mpq_patch_params));

      QLineEdit* mpq_patch_params_ledit = new QLineEdit("patch-A.MPQ", mpq_patch_params);
      QSettings settings;
      // saved last set patch name
      mpq_patch_params_ledit->setText(settings.value("noggit_window/mpq_name", "patch-A.MPQ").toString());
      mpq_patch_params_layout->addWidget(mpq_patch_params_ledit);


      QCheckBox* mpq_patch_params_locale_chk = new QCheckBox("Save DBC to Locale:", mpq_patch_params);
      mpq_patch_params_locale_chk->setToolTip("This is recommended for non English clients");
      //auto set locale mode if client isn't english.
      bool non_english = false;
      BlizzardArchive::Locale locale_mode = Noggit::Application::NoggitApplication::instance()->clientData()->locale_mode();
      if (!(locale_mode != BlizzardArchive::Locale::AUTO) && !(locale_mode != BlizzardArchive::Locale::enGB)
          && !(locale_mode != BlizzardArchive::Locale::enUS))
      {
          non_english = true;
      }
      mpq_patch_params_locale_chk->setChecked(non_english);
      // Unimplemented
      mpq_patch_params_locale_chk->setHidden(true);
      mpq_patch_params_locale_chk->setDisabled(true);
      mpq_patch_params_layout->addWidget(mpq_patch_params_locale_chk);


      // mode choice between replace archive and add files to archive
      QCheckBox* mpq_overwrite_chk = new QCheckBox("Overwrite Archive", mpq_patch_params);
      mpq_overwrite_chk->setToolTip("If there is already an archive with this name, it will be deleted and replaced by a new one.");
      mpq_overwrite_chk->setChecked(false);
      // Unimplemented
      mpq_overwrite_chk->setHidden(true);
      mpq_overwrite_chk->setDisabled(true);
      mpq_patch_params_layout->addWidget(mpq_overwrite_chk);


      QCheckBox* mpq_compact_chk = new QCheckBox("Compact Archive", mpq_patch_params);
      mpq_compact_chk->setToolTip("Adding and removing files creates empty space in the MPQ, making it grow in size.\
          \nCompacting takes a few seconds depending on patch size, it is recommended to compact when many files have been added.");
      mpq_compact_chk->setChecked(false);
      mpq_patch_params_layout->addWidget(mpq_compact_chk);

      QCheckBox* mpq_compress_files_chk = new QCheckBox("Compress Files (slow)", mpq_patch_params);
      mpq_compress_files_chk->setToolTip("Files will be compressed in the MPQ, but writting becomes much slower(about 3x slower).\
          \nRecommended for distribution.");
      mpq_compress_files_chk->setChecked(true);
      mpq_patch_params_layout->addWidget(mpq_compress_files_chk);


      QPushButton* mpq_patch_params_okay = new QPushButton("Save Project to Client MPQ", mpq_patch_params);
      mpq_patch_params_layout->addWidget(mpq_patch_params_okay);

      connect(mpq_patch_params_okay, &QPushButton::clicked
          , [=]()
          {
              // check if mpq name is allowed
              if (!Noggit::Application::NoggitApplication::instance()->clientData()->isMPQNameValid(mpq_patch_params_ledit->text().toLower().toStdString(), true))
              {
                  QMessageBox::warning(this, "Name Error", "MPQ Name is not allowed.\This name is already used by base client patches, or the client can't load it.\
                     \nYour patch must be named \"patch-[4-9].MPQ\" or \"patch-[A-Z].MPQ\".");
              }
              else
              {
                QSettings settings;
                settings.setValue("noggit_window/mpq_name", mpq_patch_params_ledit->text());
                settings.sync();

                mpq_patch_params->accept();
              }
          });

      // execute dialog, and run code when Mpq_patch_params_okay calls Mpq_patch_params->accept();
      if (mpq_patch_params->exec() == QDialog::Accepted)
      {
          // OK button was pressed, do stuff.

          auto archive_name = mpq_patch_params_ledit->text().toStdString();
          auto clientData = Noggit::Application::NoggitApplication::instance()->clientData();

          namespace fs = std::filesystem;
          // progress bar, count maximum files
          // ignore directories, only count files
          auto regular_file = [](const fs::path& path) {
              std::string const extension = path.extension().string();
              // filter noggit files
              if (extension == ".noggitproj" || extension == ".json" || extension == ".ini")
                  return false;
              return fs::is_regular_file(path);
              };

          int totalItems = std::count_if(fs::recursive_directory_iterator(clientData->projectPath()), {}, regular_file);

          if (!totalItems)
          {
              QMessageBox::warning(this, "Error", std::format("No files found in project {}", clientData->projectPath()).c_str());
              return;
          }

          if (!clientData->mpqArchiveExistsOnDisk(archive_name))
          {
              try
              {
                  auto archive = clientData->tryCreateMPQArchive(archive_name);
              }
              catch (BlizzardArchive::Exceptions::Archive::ArchiveOpenError& e)
              {
                  QMessageBox::critical(nullptr, "Error", e.what());
              }
              catch (...)
              {
                  QMessageBox::critical(nullptr, "Error", "Failed to create MPQ Archive. Unhandled exception.");
              }
          }
          {
              auto archive = clientData->getMPQArchive(archive_name);
              if (archive.has_value())
              {
                  auto progress_box = new QMessageBox(QMessageBox::Information, "Working...", std::format("Saving {} files to patch {}...\
                      \nClosing the program now can corrupt the MPQ.", std::to_string(totalItems), archive_name ).c_str(), QMessageBox::StandardButton::NoButton, this);
                  progress_box->setStandardButtons(QMessageBox::NoButton);
                  progress_box->setWindowFlags(progress_box->windowFlags() & ~Qt::WindowCloseButtonHint);
                  // progress_box->exec(); // this stops code execution
                  progress_box->repaint();
                  qApp->processEvents();

                  try
                  {
                      auto start = std::chrono::high_resolution_clock::now();

                      std::array<int, 2> result = clientData->saveLocalFilesToArchive(archive.value(), mpq_compress_files_chk->isChecked(), mpq_compact_chk->isChecked());
                      int processed_files = result[0];
                      int files_failed = processed_files - result[1];

                      auto end = std::chrono::high_resolution_clock::now();
                      std::chrono::duration<double> duration = end - start;
                      std::ostringstream oss; // duration in seconds with 1 digit
                      oss << std::fixed << std::setprecision(1) << duration.count();

                      // if no file was processed, archive was most likely opened and not accessible
                      // TODO : we can throw an error message in saveLocalFilesToArchive if (!archive->openForWritting()) instead
                      if (!processed_files)
                      {
                          QMessageBox::warning(this, "Error", "Project Folder is not a valid directory or client MPQ is not accessible.\
                              \nMake sure it isn't opened by Wow or MPQ editor");
                      }
                      else
                          QMessageBox::information(this, "Archive Updated", std::format("Added {} files to existing Archive {} in {} seconds.\
                        \n{} Files failed.",  std::to_string(processed_files), archive_name, oss.str(), std::to_string(files_failed)).c_str());


                  }
                  catch (...)
                  {
                      QMessageBox::critical(nullptr, "Error", "unhandled exception");
                  }

                  progress_box->close();
              }
              else
                  QMessageBox::warning(this, "Error", std::format("Error accessing archive {}", archive_name).c_str());
          }
      }
  }
}
