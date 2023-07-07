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

    if (project->projectVersion == Project::ProjectVersion::WOTLK)
    {
      OpenDBs(project->ClientData);
    }

    setCentralWidget(_null_widget);

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

    mysql::testConnection(true);

    if ((use_mysql && mysql::hasMaxUIDStoredDB(_world->getMapID()))
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

  void NoggitWindow::applyFilterSearch(const QString &name, int type, int expansion)
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

        QObject::connect(_line_edit_search, QOverload<const QString&>::of(&QLineEdit::textChanged), [this, _combo_search, _combo_exp_search](const QString &name)
                         {
                             applyFilterSearch(name, _combo_search->currentIndex(), _combo_exp_search->currentIndex());
                         });

        QObject::connect(_combo_search, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, _line_edit_search, _combo_exp_search](int index)
                         {
                             applyFilterSearch(_line_edit_search->text(), index, _combo_exp_search->currentIndex());
                         });

        QObject::connect(_combo_exp_search, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, _line_edit_search, _combo_search](int index)
                         {
                             applyFilterSearch(_line_edit_search->text(), _combo_search->currentIndex(), index);
                         });


        QFormLayout* _group_layout = new QFormLayout();
        _group_layout->addRow(tr("Name : "), _line_edit_search);
        _group_layout->addRow(tr("Type : "), _combo_search);
        _group_layout->addRow(tr("Expansion : "), _combo_exp_search);
        _group_search->setLayout(_group_layout);

        _first_tab_layout->addWidget(_group_search);
        _first_tab_layout->addSpacing(12);
        _first_tab_layout->addWidget(_continents_table);

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
                       check_uid_then_enter_map(pos, math::degrees(30.f), math::degrees(90.f));
                     }
    );

    auto right_side = new QTabWidget(this);

    auto minimap_holder = new QScrollArea(this);
    minimap_holder->setWidgetResizable(true);
    minimap_holder->setAlignment(Qt::AlignCenter);
    minimap_holder->setWidget(_minimap);

    right_side->addTab(minimap_holder, "Enter map");
    minimap_holder->setAccessibleName("main_menu_minimap_holder");

    _map_creation_wizard = new Noggit::Ui::Tools::MapCreationWizard::Ui::MapCreationWizard(_project, this);

    _map_wizard_connection = connect(_map_creation_wizard,
                                     &Noggit::Ui::Tools::MapCreationWizard::Ui::MapCreationWizard::map_dbc_updated, [=]
                                     {
                                       _buildMapListComponent->buildMapList(this);
                                     }
    );

    right_side->addTab(_map_creation_wizard, "Edit map");

    layout->addWidget(right_side);

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
