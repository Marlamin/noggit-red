// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MapCreationWizard.hpp"

#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include <noggit/ui/widgets/Vector3Widget.hpp>
#include <noggit/project/CurrentProject.hpp>
#include <blizzard-database-library/include/structures/Types.h>
#include <noggit/MapView.h>
#include <noggit/World.h>
#include <noggit/application/Utils.hpp>
#include <noggit/Log.h>

#include <util/qt/overload.hpp>

#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QScrollArea>
#include <QWheelEvent>
#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QStackedWidget>
#include <QDir>
#include <QMessageBox>

#include <filesystem>

using namespace Noggit::Ui::Tools::MapCreationWizard::Ui;

MapCreationWizard::MapCreationWizard(std::shared_ptr<Project::NoggitProject> project, QWidget* parent) : Noggit::Ui::widget(parent), _project(project)
{
  auto layout = new QHBoxLayout(this);

  // Left side
  auto layout_left = new QVBoxLayout(this);
  layout->addLayout(layout_left);

  auto info_label = new QLabel("Left Click on the grid to add Tiles, Ctrl+click to erase, Shift+Click to add 3x3.");
  info_label->setWindowIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::info));
  layout_left->addWidget(info_label);

  auto scroll_minimap = new QScrollArea(this);

  _minimap_widget = new Noggit::Ui::minimap_widget(this);
  _minimap_widget->draw_boundaries(true);

  layout_left->addWidget(scroll_minimap);

  scroll_minimap->setAlignment(Qt::AlignCenter);
  scroll_minimap->setWidget(_minimap_widget);
  scroll_minimap->setWidgetResizable(true);

  // Right side
  auto layout_right_holder = new QWidget(this);
  layout_right_holder->setMinimumWidth(300);
  layout_right_holder->setMaximumWidth(550);
  auto layout_right = new QVBoxLayout (layout_right_holder);
  layout_right_holder->setLayout(layout_right);
  layout->addWidget(layout_right_holder);

  auto layout_selector_wgt = new QWidget(layout_right_holder);
  auto layout_selector = new QHBoxLayout(layout_selector_wgt);
  layout_selector_wgt->setLayout(layout_selector);

  layout_selector->setAlignment(Qt::AlignLeft);
  layout_right->addWidget(layout_selector_wgt);

  _corpse_map_id = new QComboBox(this);
  _corpse_map_id->addItem("None");
  _corpse_map_id->setItemData(0, QVariant (-1));

  // Fill selector combo

  const auto& table = std::string("Map");
  auto mapTable = _project->ClientDatabase->LoadTable(table, readFileAsIMemStream);

  int count = 0;
  auto iterator = mapTable.Records();
  while (iterator.HasRecords())
  {
      auto record = iterator.Next();

      int map_id =  record.RecordId;
      std::string name = record.Columns["MapName_lang"].Value;
      int area_type = std::stoi(record.Columns["InstanceType"].Value);

      if (area_type < 0 || area_type > 4 || !World::IsEditableWorld(record))
          continue;

      _corpse_map_id->addItem(QString::number(map_id) + " - " + QString::fromUtf8(name.c_str()));
      _corpse_map_id->setItemData(count + 1, QVariant(map_id));

      count++;
  }

  _project->ClientDatabase->UnloadTable("Map");

  auto add_btn = new QPushButton("New",this);
  add_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
  layout_selector->addWidget(add_btn);

  add_btn->setAccessibleName("map_wizard_add_button");

  auto remove_btn = new QPushButton("Remove",this);
  remove_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::times));
  layout_selector->addWidget(remove_btn);
  
  remove_btn->setAccessibleName("map_wizard_remove_button");

  _map_settings = new QGroupBox("Map settings", this);
  layout_right->addWidget(_map_settings);

  auto box_map_settings_layout = new QVBoxLayout(_map_settings);
  _map_settings->setLayout(box_map_settings_layout);


  _tabs = new QTabWidget(_map_settings);

  box_map_settings_layout->addWidget(_tabs);

  createMapSettingsTab();
  createDifficultyTab();
  createWmoEntryTab();

  // Bottom row
  auto bottom_row_wgt = new QWidget(layout_right_holder);
  auto btn_row_layout = new QHBoxLayout(layout_right_holder);
  bottom_row_wgt->setLayout(btn_row_layout);
  btn_row_layout->setAlignment(Qt::AlignRight);

  auto save_btn = new QPushButton("Save", this);
  save_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::save));
  auto discard_btn = new QPushButton("Discard", this);
  btn_row_layout->addWidget(save_btn);
  btn_row_layout->addWidget(discard_btn);
  save_btn->setAccessibleName("map_wizard_save_button");
  discard_btn->setAccessibleName("map_wizard_discard_button");

  layout_right->addWidget(bottom_row_wgt);

  // Connections

  connect(save_btn, &QPushButton::clicked
      ,[&] ()
      {
        saveCurrentEntry();
      });

  connect(discard_btn, &QPushButton::clicked
      ,[&] ()
      {
        discardChanges();
      });

  connect(add_btn, &QPushButton::clicked
      ,[&] ()
          {
            addNewMap();
          });

  connect(remove_btn, &QPushButton::clicked
      ,[&] ()
          {
            removeMap();
          });

  _connection = connect(reinterpret_cast<Noggit::Ui::Windows::NoggitWindow*>(parent),
                        QOverload<int>::of(&Noggit::Ui::Windows::NoggitWindow::mapSelected)
                        , [&] (int index)
                              {
                                selectMap(index);
                              }
  );

  connect(_difficulty_type, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
      selectMapDifficulty();
      });

  // Selection

  QObject::connect
      ( _minimap_widget,  &Noggit::Ui::minimap_widget::tile_clicked
          , [this] (QPoint tile)
        {
          if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
          {
            int x = tile.x() - 1;
            int y = tile.y() - 1;

            for (int i = 0; i < 3; ++i)
            {
              for (int j = 0; j < 3; ++j)
              {
                int x_final = x + i;
                int y_final = y + j;

                if (x_final < 0 || x_final > 63 || y_final < 0 || y_final > 63)
                  continue;

                if (!_world->mapIndex.hasTile(TileIndex(x_final, y_final)))
                {
                  if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) && !_wmoEntryTab.disableTerrain->isChecked())
                  {
                    _world->mapIndex.addTile(TileIndex(x_final, y_final));
                  }
                }
                else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
                {
                  _world->mapIndex.removeTile(TileIndex(x_final, y_final));
                }
              }
            }
          }
          else
          {
            int x = tile.x();
            int y = tile.y();

            if (x < 0 || x > 63 || y < 0 || y > 63)
              return;

            if (!_world->mapIndex.hasTile(TileIndex(x, y)))
            {
              if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) && !_wmoEntryTab.disableTerrain->isChecked())
              {
                _world->mapIndex.addTile(TileIndex(x, y));
              }
            }
            else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
            {
              _world->mapIndex.removeTile(TileIndex(x, y));
            }
          }

          update();
        }
      );

}

void MapCreationWizard::createMapSettingsTab()
{
    auto map_settings_widget(new QWidget(this));
    _tabs->addTab(map_settings_widget, "Map Settings");

    auto map_settings_layout = new QFormLayout(map_settings_widget);

    _directory = new QLineEdit(_map_settings);
    map_settings_layout->addRow("Map directory:", _directory);

    _is_big_alpha = new QCheckBox(this);
    map_settings_layout->addRow("Big alpha:", _is_big_alpha);
    _is_big_alpha->setChecked(true);
    _is_big_alpha->setToolTip("Sets the alphamaps(terrain textures) precision.\nSmall alpha uses 4 bits precision, big alpha uses 8 bits.");

    _sort_by_size_cat = new QCheckBox(this);
    map_settings_layout->addRow("Sort models", _sort_by_size_cat);
    _sort_by_size_cat->setChecked(true);
    _sort_by_size_cat->setToolTip("Sorts models based on their size on save. May help increase loading speed of the map.");


    _instance_type = new QComboBox(_map_settings);
    _instance_type->addItem("None");
    _instance_type->setItemData(0, QVariant(0));

    _instance_type->addItem("Instance");
    _instance_type->setItemData(1, QVariant(1));

    _instance_type->addItem("Raid");
    _instance_type->setItemData(2, QVariant(2));

    _instance_type->addItem("Battleground");
    _instance_type->setItemData(3, QVariant(3));

    _instance_type->addItem("Arena");
    _instance_type->setItemData(4, QVariant(4));

    map_settings_layout->addRow("Map type:", _instance_type);

    _map_name = new LocaleDBCEntry(_map_settings);
    map_settings_layout->addRow("Map name:", _map_name);

    _area_table_id = new QSpinBox(_map_settings);
    map_settings_layout->addRow("Area ID:", _area_table_id);
    _area_table_id->setMaximum(std::numeric_limits<std::int32_t>::max());

    _map_desc_alliance = new LocaleDBCEntry(_map_settings);
    map_settings_layout->addRow("Description (Alliance):", _map_desc_alliance);

    _map_desc_horde = new LocaleDBCEntry(_map_settings);
    map_settings_layout->addRow("Description (Horde):", _map_desc_horde);

    _loading_screen = new QSpinBox(_map_settings);
    map_settings_layout->addRow("Loading screen:", _loading_screen);
    _loading_screen->setMaximum(std::numeric_limits<std::int32_t>::max());
    _loading_screen->setToolTip("Reference to LoadingScreens.dbc");

    _minimap_icon_scale = new QDoubleSpinBox(_map_settings);
    map_settings_layout->addRow("Minimap icon scale:", _minimap_icon_scale);

    _corpse_map_id->setCurrentText("None");
    map_settings_layout->addRow("Corpse map:", _corpse_map_id);
    _corpse_map_id->setToolTip("Map in which player will spawn as a ghost.Used for dungeons spawning dead players in continents."
                               "\nSet to -1 to spawn in this map.");

    _corpse_x = new QDoubleSpinBox(_map_settings);
    map_settings_layout->addRow("Corpse X:", _corpse_x);
    _corpse_x->setMinimum(-17066.66656); // map size
    _corpse_x->setMaximum(17066.66656);

    _corpse_y = new QDoubleSpinBox(_map_settings);
    map_settings_layout->addRow("Corpse Y:", _corpse_y);
    _corpse_y->setMinimum(-17066.66656); // map size
    _corpse_y->setMaximum(17066.66656);

    _time_of_day_override = new QSpinBox(_map_settings);
    _time_of_day_override->setMinimum(-1);
    _time_of_day_override->setMaximum(2880); // Time Values from 0 to 2880 where each number represents a half minute from midnight to midnight 
    _time_of_day_override->setValue(-1);
    _time_of_day_override->setToolTip("Override map time to a static time. Set to -1 for dynamic (normal) time."
      "Time Values from 0 to 2880 where each number represents a half minute from midnight to midnight.");

    map_settings_layout->addRow("Daytime override:", _time_of_day_override);

    _expansion_id = new QComboBox(_map_settings);

    _expansion_id->addItem("Classic");
    _expansion_id->setItemData(0, QVariant(0));

    _expansion_id->addItem("Burning Crusade");
    _expansion_id->setItemData(1, QVariant(1));

    _expansion_id->addItem("Wrath of the Lich King");
    _expansion_id->setItemData(2, QVariant(2));

    map_settings_layout->addRow("Expansion:", _expansion_id);

    _raid_offset = new QSpinBox(_map_settings);
    _raid_offset->setMaximum(std::numeric_limits<std::int32_t>::max());
    map_settings_layout->addRow("Raid offset:", _raid_offset);
    _raid_offset->setToolTip("Unknown. Used only by Onyxia and ZG.");

    _max_players = new QSpinBox(_map_settings);
    _max_players->setMaximum(std::numeric_limits<std::int32_t>::max());
    map_settings_layout->addRow("Max players:", _max_players);
}

void MapCreationWizard::createDifficultyTab()
{
    auto map_difficulty_widget(new QWidget(this));
    _tabs->addTab(map_difficulty_widget, "Map Difficulty Settings");

    auto difficulty_settings_layout = new QFormLayout(map_difficulty_widget);
    _map_settings->setLayout(difficulty_settings_layout);

    _difficulty_type = new QComboBox(_map_settings);
    _difficulty_type->setDisabled(true);

    difficulty_settings_layout->addRow("Difficulty Index", _difficulty_type);

    _difficulty_req_message = new LocaleDBCEntry(_map_settings);
    _difficulty_req_message->setDisabled(true); // disable them until they're actually saveable, only "display" it for now
    difficulty_settings_layout->addRow("Requirement Message", _difficulty_req_message);

    _difficulty_raid_duration = new QSpinBox(_map_settings);
    _difficulty_raid_duration->setDisabled(true);
    _difficulty_raid_duration->setRange(0, 7);
    difficulty_settings_layout->addRow("Instance Duration(days)", _difficulty_raid_duration);

    _difficulty_max_players = new QSpinBox(_map_settings);
    _difficulty_max_players->setDisabled(true);
    difficulty_settings_layout->addRow("Max Players", _difficulty_max_players);

    _difficulty_string = new QLineEdit(_map_settings);
    _difficulty_string->setDisabled(true);
    difficulty_settings_layout->addRow("Difficulty String", _difficulty_string);

    difficulty_settings_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void MapCreationWizard::createWmoEntryTab()
{
    auto wmo_widget(new QWidget(this));
    _tabs->addTab(wmo_widget, "WMO Entry");

    auto layout = new QFormLayout(wmo_widget);
    _map_settings->setLayout(layout);

    _wmoEntryTab.disableTerrain = new QCheckBox(_map_settings);
    _wmoEntryTab.disableTerrain->setChecked(false);
    connect(_wmoEntryTab.disableTerrain, &QCheckBox::toggled, [=](bool state) {
        _wmoEntryTab.wmoPath->setDisabled(!state);
        // Is there any value in exposing these?
        //_wmoEntryTab.nameId->setDisabled(!state);
        //_wmoEntryTab.uniqueId->setDisabled(!state);
        _wmoEntryTab.position->setDisabled(!state);
        _wmoEntryTab.rotation->setDisabled(!state);
        //_wmoEntryTab.flags->setDisabled(!state);
        _wmoEntryTab.doodadSet->setDisabled(!state);
        _wmoEntryTab.nameSet->setDisabled(!state);
        });
    layout->addRow("WMO only (disable terrain):", _wmoEntryTab.disableTerrain);

    _wmoEntryTab.wmoPath = new QLineEdit(_map_settings);
    _wmoEntryTab.wmoPath->setDisabled(true);
    auto defaultStylesheet = _wmoEntryTab.wmoPath->styleSheet();
    connect(_wmoEntryTab.wmoPath, &QLineEdit::textChanged, [=](QString text) {
        if (!_wmoEntryTab.disableTerrain->isChecked())
        {
            return;
        }

        _world->mWmoFilename = text.toStdString();
        WMOInstance wmo{ _world->mWmoFilename, _world->getRenderContext() };
        wmo.wmo->wait_until_loaded();
        if (!wmo.finishedLoading() || wmo.wmo->loading_failed())
        {
            _wmoEntryTab.wmoPath->setStyleSheet("QLineEdit { background-color : red; }");
            return;
        }

        _wmoEntryTab.wmoEntry.extents = wmo.getExtents();
        _wmoEntryTab.wmoPath->setStyleSheet(defaultStylesheet);
        populateDoodadSet(wmo);
        populateNameSet(wmo);
        });
    layout->addRow("Path:", _wmoEntryTab.wmoPath);

    _wmoEntryTab.nameId = new QSpinBox(_map_settings);
    _wmoEntryTab.nameId->setDisabled(true);
    _wmoEntryTab.nameId->setMinimum(std::numeric_limits<std::int32_t>::min());
    _wmoEntryTab.nameId->setMaximum(std::numeric_limits<std::int32_t>::max()); 
    connect(_wmoEntryTab.nameId, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int value) {
        _wmoEntryTab.wmoEntry.nameID = value;
        });
    layout->addRow("Name Id:", _wmoEntryTab.nameId);

    _wmoEntryTab.uniqueId = new QSpinBox(_map_settings);
    _wmoEntryTab.uniqueId->setDisabled(true);
    _wmoEntryTab.uniqueId->setMinimum(std::numeric_limits<std::int32_t>::min());
    _wmoEntryTab.uniqueId->setMaximum(std::numeric_limits<std::int32_t>::max());
    connect(_wmoEntryTab.uniqueId, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int value) {
        _wmoEntryTab.wmoEntry.uniqueID = value;
        });
    layout->addRow("Unique Id:", _wmoEntryTab.uniqueId);

    _wmoEntryTab.position = new Vector3fWidget(_map_settings);
    _wmoEntryTab.position->setDisabled(true);
    connect(_wmoEntryTab.position, &Vector3fWidget::valueChanged , [=](glm::vec3 const& value) {
        _wmoEntryTab.wmoEntry.pos[0] = value.x;
        _wmoEntryTab.wmoEntry.pos[1] = value.y;
        _wmoEntryTab.wmoEntry.pos[2] = value.z;
        });
    layout->addRow("Position:", _wmoEntryTab.position);

    _wmoEntryTab.rotation = new Vector3fWidget(_map_settings);
    _wmoEntryTab.rotation->setDisabled(true);
    connect(_wmoEntryTab.rotation, &Vector3fWidget::valueChanged, [=](glm::vec3 const& value) {
        _wmoEntryTab.wmoEntry.rot[0] = value.x;
        _wmoEntryTab.wmoEntry.rot[1] = value.y;
        _wmoEntryTab.wmoEntry.rot[2] = value.z;
        });
    layout->addRow("Rotation:", _wmoEntryTab.rotation);

    _wmoEntryTab.flags = new QSpinBox(_map_settings);
    _wmoEntryTab.flags->setDisabled(true);
    _wmoEntryTab.flags->setMinimum(std::numeric_limits<std::uint16_t>::min());
    _wmoEntryTab.flags->setMaximum(std::numeric_limits<std::uint16_t>::max());
    connect(_wmoEntryTab.flags, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int value) {
        _wmoEntryTab.wmoEntry.flags = static_cast<uint16_t>(value);
        });
    layout->addRow("Flags:", _wmoEntryTab.flags);

    _wmoEntryTab.doodadSet = new QComboBox(_map_settings);
    _wmoEntryTab.doodadSet->setDisabled(true);
    connect(_wmoEntryTab.doodadSet, &QComboBox::currentTextChanged, [=](QString)
        {
            _wmoEntryTab.wmoEntry.doodadSet = _wmoEntryTab.doodadSet->currentIndex();
        });
    layout->addRow("Doodad Set:", _wmoEntryTab.doodadSet);

    _wmoEntryTab.nameSet = new QComboBox(_map_settings);
    _wmoEntryTab.nameSet->setDisabled(true);
    connect(_wmoEntryTab.nameSet, &QComboBox::currentTextChanged, [=](QString)
        {
            _wmoEntryTab.wmoEntry.nameSet = _wmoEntryTab.nameSet->currentIndex();
        });
    layout->addRow("Name set:", _wmoEntryTab.nameSet);
}

void MapCreationWizard::populateWmoEntryTab()
{
    _wmoEntryTab.disableTerrain->setChecked(_world->mapIndex.hasAGlobalWMO());
    if (!_world->mapIndex.hasAGlobalWMO())
    {
        _wmoEntryTab.wmoPath->clear();
        _wmoEntryTab.nameId->clear();
        _wmoEntryTab.uniqueId->clear();
        _wmoEntryTab.position->clear();
        _wmoEntryTab.rotation->clear();
        _wmoEntryTab.flags->clear();
        _wmoEntryTab.doodadSet->clear();
        _wmoEntryTab.nameSet->clear();
    }
    else
    {
        _wmoEntryTab.wmoPath->setText(QString::fromStdString(_world->mWmoFilename));
        _wmoEntryTab.wmoEntry = _world->mWmoEntry;
        _wmoEntryTab.nameId->setValue(_wmoEntryTab.wmoEntry.nameID);
        _wmoEntryTab.uniqueId->setValue(_wmoEntryTab.wmoEntry.uniqueID);
        _wmoEntryTab.position->setValue(_wmoEntryTab.wmoEntry.pos);
        _wmoEntryTab.rotation->setValue(_wmoEntryTab.wmoEntry.rot);
        _wmoEntryTab.flags->setValue(_wmoEntryTab.wmoEntry.flags);
        _wmoEntryTab.doodadSet->setCurrentIndex(_wmoEntryTab.wmoEntry.doodadSet);
        _wmoEntryTab.nameSet->setCurrentIndex(_wmoEntryTab.wmoEntry.nameSet);
    }
}

void MapCreationWizard::populateDoodadSet(WMOInstance& instance)
{
    QSignalBlocker const doodadsetblocker(_wmoEntryTab.doodadSet);
    _wmoEntryTab.doodadSet->clear();

    QStringList doodadsetnames;
    for (auto& doodad_set : instance.wmo->doodadsets)
    {
        doodadsetnames.append(doodad_set.name);
    }
    _wmoEntryTab.doodadSet->insertItems(0, doodadsetnames);
    _wmoEntryTab.doodadSet->setCurrentIndex(instance.doodadset());
}

void MapCreationWizard::populateNameSet(WMOInstance& instance)
{
    // get names from WMOAreatable, if no name, get from areatable
    // if no areatable, we have to get the terrain's area
    QSignalBlocker const namesetblocker(_wmoEntryTab.nameSet);
    _wmoEntryTab.nameSet->clear();
    auto wmoid = instance.wmo->WmoId;
    auto setnames = gWMOAreaTableDB.getWMOAreaNames(wmoid);
    QStringList namesetnames;
    for (auto& area_name : setnames)
    {
        if (area_name.empty())
        {
            auto chunk = _world->getChunkAt(instance.pos);
            namesetnames.append(gAreaDB.getAreaFullName(chunk->getAreaID()).c_str());
        }
        else
            namesetnames.append(area_name.c_str());
    }

    _wmoEntryTab.nameSet->insertItems(0, namesetnames);
    _wmoEntryTab.nameSet->setCurrentIndex(instance.mNameset);
}

std::string MapCreationWizard::getDifficultyString()
{
  int instance_type = _instance_type->itemData(_instance_type->currentIndex()).toInt();
  int difficulty_type = _difficulty_type->currentIndex();
  assert(instance_type == 1 || instance_type == 2);

/*
| Name               | Entry Condition      | Difficulty Entry 1      | Difficulty Entry 2      | Difficulty Entry 3      |
|--------------------|----------------------|-------------------------|-------------------------|-------------------------|
| Normal Creature    | Different than 0     | 0                       | 0                       | 0                       |
| Dungeon Creature   | Normal Dungeon       | Heroic Dungeon          | 0                       | 0                       |
| Raid Creature      | 10man Normal Raid    | 25man Normal Raid       | 10man Heroic Raid       | 25man Heroic Raid       |
| Battleground       | 51-59                | 60-69                   | 70-79                   | 80                      |
*/

    if (instance_type == 1 && _difficulty_max_players->value() == 5) // dungeon
    {
      if (difficulty_type == 0)
        return "DUNGEON_DIFFICULTY_5PLAYER";
      else if (difficulty_type == 1)
        return "DUNGEON_DIFFICULTY_5PLAYER_HEROIC";
      else
        return "Unsupported difficulty for 5 men dungeon";
    }
    else if (instance_type == 2) // raid
    {
        switch (_difficulty_max_players->value())
        {
        case 10:
            if (difficulty_type == 0)
                return "RAID_DIFFICULTY_10PLAYER";
            else if (difficulty_type == 2)
                return "RAID_DIFFICULTY_10PLAYER_HEROIC";
            break;
        case 20:
            if (difficulty_type == 0)
                return "RAID_DIFFICULTY_20PLAYER";
            break;
        case 25:
            // in BC 25men was difficulty 0, after the 10men mode in wrath it is difficulty 1
            if (difficulty_type == 0 || difficulty_type == 1) // maybe instead check if a difficulty 25 already exists
                return "RAID_DIFFICULTY_25PLAYER";
            else if (difficulty_type == 3)
                return "RAID_DIFFICULTY_25PLAYER_HEROIC";
            break;
        case 40:
          if (difficulty_type == 0)
            return "RAID_DIFFICULTY_40PLAYER";
          break;
        default:
          return "invalid player count";
        }
    }
    assert(false);
    return "";
}

void MapCreationWizard::selectMap(int map_id)
{
  _is_new_record = false;

  // int map_id = world->getMapID();

  auto table = _project->ClientDatabase->LoadTable("Map", readFileAsIMemStream);
  auto record = table.Record(map_id);

  _cur_map_id = map_id;

  if (_world)
  {
    // delete _world;
    _world.reset();
  }

  // auto noggitWindow = reinterpret_cast<Noggit::Ui::Windows::NoggitWindow*>(parent());
  // _world = world;


  auto directoryName = record.Columns["Directory"].Value;
  auto instanceType = record.Columns["InstanceType"].Value;

  auto areaTableId = record.Columns["AreaTableID"].Value;
  auto loadingScreenId = record.Columns["LoadingScreenID"].Value;
  auto minimapIconScale = record.Columns["MinimapIconScale"].Value;
  auto corpseMapId = record.Columns["CorpseMapID"].Value;
  auto corpseCoords = record.Columns["Corpse"].Values;
  auto expansionId = record.Columns["ExpansionID"].Value;
  auto maxPlayers = record.Columns["MaxPlayers"].Value;
  auto timeOfDayOverride = record.Columns["TimeOfDayOverride"].Value;
  // auto timeOffset = record.Columns["TimeOffset"].Value;
  auto raidOffset = record.Columns["RaidOffset"].Value;

  // _world = new World(directoryName, map_id, Noggit::NoggitRenderContext::MAP_VIEW);
  _world = std::make_unique<World>(directoryName, map_id, Noggit::NoggitRenderContext::MAP_VIEW);

  // check if map has a wdl and prompt to create a new one
  std::stringstream filename;
  filename << "World\\Maps\\" << _world->basename << "\\" << _world->basename << ".wdl";
  if (!Application::NoggitApplication::instance()->clientData()->exists(filename.str()))
  {
     QMessageBox prompt;
     prompt.setText(std::string("This map has no existing horizon data (.wdl file).").c_str());
     prompt.setInformativeText(std::string("Do you want to generate a new .wdl file ?").c_str());
     prompt.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
     prompt.setDefaultButton(QMessageBox::Yes);
     bool answer = prompt.exec() == QMessageBox::StandardButton::Yes;
     if (answer)
     {
        _world->horizon.save_wdl(_world.get(), true);
        _world->horizon.set_minimap(&_world->mapIndex);
        // _world = new World(directoryName, map_id, Noggit::NoggitRenderContext::MAP_VIEW); // refresh minimap
     }
  }

  _minimap_widget->world(_world.get());

  _directory->setText(QString::fromStdString(directoryName));
  _directory->setEnabled(false);

  _is_big_alpha->setChecked(_world->mapIndex.hasBigAlpha());
  _is_big_alpha->setEnabled(false);

  _sort_by_size_cat->setChecked(_world->mapIndex.sort_models_by_size_class());

  _instance_type->setCurrentIndex(std::atoi(instanceType.c_str()));

  _map_name->fill(record, "MapName_lang");
  _map_desc_alliance->fill(record, "MapDescription1_lang");
  _map_desc_horde->fill(record, "MapDescription0_lang");

  _area_table_id->setValue(std::atoi(areaTableId.c_str()));

  _loading_screen->setValue(std::atoi(loadingScreenId.c_str()));
  _minimap_icon_scale->setValue(std::atof(minimapIconScale.c_str()));

  int corpse_map_idx = std::atoi(corpseMapId.c_str());
  for (int i = 0; i < _corpse_map_id->count(); ++i)
  {
    if (_corpse_map_id->itemData(i) == corpse_map_idx)
    {
      _corpse_map_id->setCurrentIndex(i);
    }
  }

  if(corpseCoords.size() > 0)
  {
      _corpse_x->setValue(std::atoi(corpseCoords[0].c_str()));
      _corpse_y->setValue(std::atoi(corpseCoords[1].c_str()));
  }

  _time_of_day_override->setValue(std::atoi(timeOfDayOverride.c_str()));
  _expansion_id->setCurrentIndex(std::atoi(expansionId.c_str()));

  _raid_offset->setValue(std::atoi(raidOffset.c_str())); // only ever used in 2 places? not sure what for

  _max_players->setValue(std::atoi(maxPlayers.c_str()));

  _project->ClientDatabase->UnloadTable("Map");

  auto difficulty_table = _project->ClientDatabase->LoadTable("MapDifficulty", readFileAsIMemStream);

  auto iterator = difficulty_table.Records();

  QSignalBlocker const difficulty_type_blocker(_difficulty_type);
  _difficulty_type->clear();

  while (iterator.HasRecords())
  {
      auto record = iterator.Next();

      // auto difficulty_id = std::atoi(record.Columns["ID"].Value.c_str());
      auto record_id = record.RecordId;
      auto diff_mapId = std::atoi(record.Columns["MapID"].Value.c_str());
      auto difficulty_type = std::atoi(record.Columns["Difficulty"].Value.c_str());
      if (diff_mapId == map_id)
      {
          std::string diff_text = "Difficulty " + record.Columns["Difficulty"].Value;
          _difficulty_type->insertItem(difficulty_type, diff_text.c_str(), QVariant(record_id));
      }
  }
  _project->ClientDatabase->UnloadTable("MapDifficulty");
  _difficulty_type->setCurrentIndex(0);
  selectMapDifficulty();

  populateWmoEntryTab();
}

void MapCreationWizard::selectMapDifficulty()
{
    if (!_difficulty_type->count())
        return;

    auto selected_difficulty_id = _difficulty_type->itemData(_difficulty_type->currentIndex()).toInt();
    if (!selected_difficulty_id)
        return;

    auto difficulty_table = _project->ClientDatabase->LoadTable("MapDifficulty", readFileAsIMemStream);
    auto record = difficulty_table.Record(selected_difficulty_id);

    //_difficulty_type;
    _difficulty_req_message->fill(record, "Message_lang");
    
    auto raid_duration = std::atoi(record.Columns["RaidDuration"].Value.c_str());
    _difficulty_raid_duration->setValue(raid_duration / 60 / 60 / 24); // convert from seconds to days

    _difficulty_max_players->setValue(std::atoi(record.Columns["MaxPlayers"].Value.c_str()));
    _difficulty_string->setText(record.Columns["Difficultystring"].Value.c_str());

    _project->ClientDatabase->UnloadTable("MapDifficulty");
}

void MapCreationWizard::wheelEvent(QWheelEvent* event)
{

  if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
  {
    const int degrees = event->angleDelta().y() / 8;
    int steps = degrees / 15;

    auto base_size = _minimap_widget->width();

    if (steps > 0)
    {
      auto new_size = std::max(512, base_size + 64);
      _minimap_widget->setFixedSize(new_size, new_size);
    } else
    {
      auto new_size = std::min(4096, base_size - 64);
      _minimap_widget->setFixedSize(new_size, new_size);
    }

    event->ignore();
  }

}

void MapCreationWizard::saveCurrentEntry()
{

  if (_is_new_record)
  {
    auto map_internal_name = _directory->text().toStdString();

    // Check if map with that name already exists
    int id = gMapDB.findMapName(map_internal_name);

    if (id >= 0)
    {
      QMessageBox prompt;
      prompt.setIcon (QMessageBox::Warning);
      prompt.setWindowFlags(Qt::WindowStaysOnTopHint);
      prompt.setText ("Warning!");
      prompt.setInformativeText ("Saving failed. The map with this directory name already exists. Rename it to save.");
      prompt.addButton ("Okay", QMessageBox::AcceptRole);
      prompt.setWindowFlags (Qt::CustomizeWindowHint | Qt::WindowTitleHint);

      prompt.exec();
      return;
    }
    // prompt the user if no tile was select
    if (!_world->mapIndex.getNumExistingTiles()) // _world->mapIndex.hasAGlobalWMO()
    {
        QMessageBox prompt;
        prompt.setIcon(QMessageBox::Warning);
        prompt.setWindowTitle("Empty map");
        prompt.setText(std::string("Warning : Your map is empty.").c_str());
        prompt.setInformativeText("You didn't set any tile. To add tiles to your map click on the squares in the grid.\nDo you still want to save the empty map ?");
        prompt.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
        prompt.setDefaultButton(QMessageBox::No);
        prompt.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
        bool answer = prompt.exec() == QMessageBox::StandardButton::Yes;
        if (!answer)
        {
            return;
        }
    }

    // Create WDT empty file for new map
    std::stringstream filename;
    filename << "World\\Maps\\" << map_internal_name << "\\" << map_internal_name << ".wdt";

    auto project_path = std::filesystem::path(Noggit::Project::CurrentProject::get()->ProjectPath.c_str());

    QDir dir((project_path / "/world/maps/" / map_internal_name).string().c_str());

    if (!dir.exists())
      dir.mkpath(".");

    auto filepath = project_path / BlizzardArchive::ClientData::normalizeFilenameInternal (filename.str());

    QFile file(filepath.string().c_str());
    file.open(QIODevice::WriteOnly);
    file.close();
  }

  // Save ADTs and WDT to disk
  // _world->mapIndex.create_empty_wdl();
  _world->mapIndex.setBigAlpha(_is_big_alpha->isChecked());
  _world->setBasename(_directory->text().toStdString());
  _world->mapIndex.set_sort_models_by_size_class(_sort_by_size_cat->isChecked());
  if (_wmoEntryTab.disableTerrain->isChecked())
  {
      _world->mapIndex.addGlobalWmo(_wmoEntryTab.wmoPath->text().toStdString(), _wmoEntryTab.wmoEntry);
  }
  else
  {
      _world->mapIndex.removeGlobalWmo();
  }
  _world->mapIndex.saveChanged(_world.get(), true);
  _world->mapIndex.save(); // save wdt file
  
  if (_is_new_record)
  {
    _world->mapIndex.create_empty_wdl(); // create default wdl

    // save default maxguid to avoid the uid fix popup
    _world->mapIndex.saveMaxUID();

    // TODO save mapdifficulty.dbc
    //

    // save default global light.dbc entry for new maps
    try
    {
        int new_id = gLightDB.getEmptyRecordID();
        DBCFile::Record record = gLightDB.addRecord(new_id);
        record.write(LightDB::Map, _cur_map_id);
        // positions and falloffs should be defaulted to 0
        // set some default light params to the same as eastern kingdom
        record.write(LightDB::DataIDs + 0, 12);// SKY_PARAM_CLEAR
        record.write(LightDB::DataIDs + 1, 13);//     CLEAR_WATER,
        record.write(LightDB::DataIDs + 2, 10);//     STORM,
        record.write(LightDB::DataIDs + 3, 13);//     STORM_WATER,
        record.write(LightDB::DataIDs + 4, 4);//     DEATH,

        gLightDB.save();
    }
    catch (LightDB::AlreadyExists)
    {
        assert(false);
        LogError << "Light.dbc entry already exists, failed to add record" << std::endl;
    }
  }

  // Save Map.dbc record
  try
  {
    DBCFile::Record record = _is_new_record ? gMapDB.addRecord(_cur_map_id) : gMapDB.getByID(_cur_map_id);

    record.writeString(MapDB::InternalName, _directory->text().toStdString());

    record.write(MapDB::AreaType, _instance_type->itemData(_instance_type->currentIndex()).toInt());
    record.write(MapDB::Flags, _sort_by_size_cat->isChecked() ? 16 : 0 );
    _map_name->toRecord(record, MapDB::Name);

    record.write(MapDB::AreaTableID, _area_table_id->value());
    _map_desc_alliance->toRecord(record, MapDB::MapDescriptionAlliance);
    _map_desc_horde->toRecord(record, MapDB::MapDescriptionHorde);
    record.write(MapDB::LoadingScreen, _loading_screen->value());
    record.write(MapDB::minimapIconScale, static_cast<float>(_minimap_icon_scale->value()));
    record.write(MapDB::corpseMapID, _corpse_map_id->itemData(_corpse_map_id->currentIndex()).toInt());
    record.write(MapDB::corpseX, static_cast<float>(_corpse_x->value()));
    record.write(MapDB::corpseY, static_cast<float>(_corpse_y->value()));
    record.write(MapDB::TimeOfDayOverride, _time_of_day_override->value());
    record.write(MapDB::ExpansionID, _expansion_id->itemData(_expansion_id->currentIndex()).toInt());
    record.write(MapDB::RaidOffset, _raid_offset->value());
    record.write(MapDB::NumberOfPlayers, _max_players->value());

    gMapDB.save();

    // reloads map list, and selects the new map
    emit map_dbc_updated(_cur_map_id);

    _is_new_record = false;
  }
  catch (MapDB::AlreadyExists)
  {
      QMessageBox::information(this
          , "Error"
          , QString("A map with Id %1 already exists").arg(_cur_map_id)
          , QMessageBox::Ok
      );
  }
  catch (MapDB::NotFound)
  {
      LogError << "Map.dbc entry " << _cur_map_id << " was not found" << std::endl;

      QMessageBox::information(this
          , "Error"
          , QString("Map.dbc entry %1 was not found").arg(_cur_map_id)
          , QMessageBox::Ok
      );
  }
}

void MapCreationWizard::discardChanges()
{
  if (!_is_new_record)
  {
    selectMap(_cur_map_id);
  }
  else
  {
    addNewMap();
  }

}

MapCreationWizard::~MapCreationWizard()
{
  // delete _world;
  disconnect(_connection);
}

void MapCreationWizard::addNewMap()
{
  _is_new_record = true;
  _cur_map_id = gMapDB.getEmptyRecordID();

  if (_world)
  {
    // delete _world;
    _world.reset();
  }

  // default to a new internal map name that isn't already used, or default world will load existing files
  std::string const base_name = "New_Map";
  std::string internal_map_name = base_name;
  int id = gMapDB.findMapName(internal_map_name);

  int suffix = 1;
  while (id >= 0) {
      internal_map_name = base_name + std::to_string(suffix);  // Append the current suffix to the base name
      id = gMapDB.findMapName(internal_map_name);  // Check if the name exists
      suffix++;
  }

  // _world = new World(internal_map_name, _cur_map_id, Noggit::NoggitRenderContext::MAP_VIEW, true);
  _world = std::make_unique<World>(internal_map_name, _cur_map_id, Noggit::NoggitRenderContext::MAP_VIEW, true);

  // hack to reset the minimap if there is an existing WDL with the same path(happens when removing a map from map.dbc but not the files
  _world->horizon.set_minimap(&_world->mapIndex, true);

  _minimap_widget->world(_world.get());

  _directory->setText(internal_map_name.c_str());
  _directory->setEnabled(true);

  _is_big_alpha->setChecked(true);
  _is_big_alpha->setEnabled(true);

  _sort_by_size_cat->setChecked(true);

  _instance_type->setCurrentIndex(0);

  _map_name->setDefaultLocValue("Unnamed Noggit Map");
  _area_table_id->setValue(0);

  _map_desc_alliance->clear();
  _map_desc_horde->clear();

  _loading_screen->setValue(0);
  _minimap_icon_scale->setValue(1.0f);

  _corpse_map_id->setCurrentIndex(0);


  _corpse_x->setValue(0.0f);
  _corpse_y->setValue(0.0f);
  _time_of_day_override->setValue(-1);
  _expansion_id->setCurrentIndex(0);
  _raid_offset->setValue(0);
  _max_players->setValue(0);
}

void MapCreationWizard::removeMap()
{
  QMessageBox prompt;
  prompt.setIcon(QMessageBox::Warning);
  prompt.setWindowTitle("Remove Map");
  prompt.setText(std::string("Are you sure you want to remove this map ?").c_str());
  prompt.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
  prompt.setDefaultButton(QMessageBox::No);
  prompt.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
  bool answer = prompt.exec() == QMessageBox::StandardButton::Yes;
  if (!answer)
  {
      return;
  }

  if (!_is_new_record && _cur_map_id >= 0)
  {
    gMapDB.removeRecord(_cur_map_id);
    gMapDB.save();
  }

  emit map_dbc_updated();

  addNewMap();
}


LocaleDBCEntry::LocaleDBCEntry(QWidget* parent) : QWidget(parent)
{
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  _show_entry = new QStackedWidget(this);

  _en = new QLineEdit(this);
  _kr = new QLineEdit(this);
  _fr = new QLineEdit(this);
  _de = new QLineEdit(this);
  _cn = new QLineEdit(this);
  _tw = new QLineEdit(this);
  _es = new QLineEdit(this);
  _mx = new QLineEdit(this);
  _ru = new QLineEdit(this);
  _jp = new QLineEdit(this);
  _pt = new QLineEdit(this);
  _it = new QLineEdit(this);

  _unk1 = new QLineEdit(this);
  _unk2 = new QLineEdit(this);
  _unk3 = new QLineEdit(this);
  _unk4 = new QLineEdit(this);

  _flags = new QSpinBox(this);
  _flags->setVisible(false);

  _show_entry->addWidget(_en);
  _show_entry->addWidget(_kr);
  _show_entry->addWidget(_fr);
  _show_entry->addWidget(_de);
  _show_entry->addWidget(_cn);
  _show_entry->addWidget(_tw);
  _show_entry->addWidget(_es);
  _show_entry->addWidget(_mx);
  _show_entry->addWidget(_ru);
  _show_entry->addWidget(_jp);
  _show_entry->addWidget(_pt);
  _show_entry->addWidget(_it);
  _show_entry->addWidget(_unk1);
  _show_entry->addWidget(_unk2);
  _show_entry->addWidget(_unk3);
  _show_entry->addWidget(_unk4);

  layout->addWidget(_show_entry);

  _current_locale = new QComboBox(this);

  for (auto const &loc : _locale_names)
  {
    _current_locale->addItem(QString::fromStdString(loc));
  }

  _widget_map = {

      {"enUS", _en},
      {"koKR", _kr},
      {"frFR", _fr},
      {"deDE", _de},
      {"zhCN", _cn},
      {"zhTW", _tw},
      {"esES", _es},
      {"esMX", _mx},
      {"ruRU", _ru},
      {"jaJP", _jp},
      {"ptPT", _pt},
      {"itIT", _it},
      {"Unknown 1", _unk1},
      {"Unknown 2", _unk2},
      {"Unknown 3", _unk3},
      {"Unknown 4", _unk4}
  };

  layout->addWidget(_current_locale);

  int locale_id = Noggit::Application::NoggitApplication::instance()->clientData()->getLocaleId();
  _current_locale->setCurrentIndex(locale_id);
  setCurrentLocale(_locale_names[locale_id]);

  // Connect

  connect ( _current_locale, &QComboBox::currentTextChanged
      , [&] (QString s)
            {
              setCurrentLocale(_current_locale->currentText().toStdString());
            }
  );

  setMaximumHeight(_en->height());
}

void LocaleDBCEntry::setCurrentLocale(const std::string& locale)
{
  _show_entry->setCurrentWidget(_widget_map.at(locale));
}

void LocaleDBCEntry::fill(DBCFile::Record& record, size_t field)
{
  for (int loc = 0; loc < 16; ++loc)
  {
    setValue(record.getLocalizedString(field, loc), loc);
  }

  _flags->setValue(record.getInt(field + 16));
}

void LocaleDBCEntry::fill(BlizzardDatabaseLib::Structures::BlizzardDatabaseRow& record, std::string columnName)
{
    auto columnValues = record.Columns[columnName].Values;
    auto columnFlagsValue = record.Columns[columnName + "_flags"].Value;

    if(columnValues.size() == 0)
    {
        auto singleValue = record.Columns[columnName].Value;
        setValue(singleValue, 0);
    }
    else
    {
        for (int loc = 0; loc < 16; ++loc)
        {
            setValue(columnValues[loc], loc);
        }
    }

    _flags->setValue(std::atoi(columnFlagsValue.c_str()));
}

void LocaleDBCEntry::toRecord(DBCFile::Record &record, size_t field)
{
  for (int loc = 0; loc < 16; ++loc)
  {
    record.writeLocalizedString(field,getValue(loc), loc);
  }

  record.write(field + 16, _flags->value());
}

void LocaleDBCEntry::setDefaultLocValue(const std::string& text)
{
  // set the default locale's widget text and select it, but don't write data.

  int locale_id = Noggit::Application::NoggitApplication::instance()->clientData()->getLocaleId();
  _current_locale->setCurrentIndex(locale_id);
  setCurrentLocale(_locale_names[locale_id]);

  // fill default locale's line edit
  setValue(text, locale_id);
}

void LocaleDBCEntry::clear()
{
  for (int loc = 0; loc < 16; ++loc)
  {
    setValue("", loc);
  }

  _flags->setValue(0);
}



