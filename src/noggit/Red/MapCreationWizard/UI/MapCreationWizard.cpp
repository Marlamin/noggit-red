// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MapCreationWizard.hpp"

#include <noggit/ui/font_awesome.hpp>
#include <noggit/MapView.h>
#include <noggit/World.h>
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
#include <QStackedWidget>

using namespace noggit::Red::MapCreationWizard::Ui;

MapCreationWizard::MapCreationWizard(QWidget* parent) : noggit::ui::widget(parent)
{
  setWindowTitle ("Map Creation Wizard");
  setWindowIcon (QIcon (":/icon"));
  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

  //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto layout = new QHBoxLayout(this);

  // Left side
  auto layout_left = new QFormLayout (this);
  layout->addLayout(layout_left);

  auto scroll_minimap = new QScrollArea(this);
  scroll_minimap->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  _minimap_widget = new noggit::ui::minimap_widget(this);
  _minimap_widget->draw_boundaries(true);

  layout_left->addWidget(scroll_minimap);

  scroll_minimap->setAlignment(Qt::AlignCenter);
  scroll_minimap->setWidget(_minimap_widget);
  scroll_minimap->setWidgetResizable(true);
  scroll_minimap->setFixedSize(QSize(512, 512));

  // Right side
  auto layout_right = new QVBoxLayout (this);
  layout->addLayout(layout_right);

  auto layout_selector = new QHBoxLayout(this);
  layout_right->addItem(layout_selector);

  _selected_map = new QComboBox(this);
  layout_selector->addWidget(new QLabel("Map:"));
  layout_selector->addWidget(_selected_map);

  _corpse_map_id = new QComboBox(this);
  _corpse_map_id->addItem("None");
  _corpse_map_id->setItemData(0, QVariant (-1));

  // Fill selector combo
  int count = 0;
  for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
  {
    int map_id = i->getInt(MapDB::MapID);
    std::string name = i->getLocalizedString(MapDB::Name);
    int area_type = i->getUInt(MapDB::AreaType);

    if (area_type < 0 ||area_type > 4 || !World::IsEditableWorld(map_id))
      continue;

    _selected_map->addItem(QString::number(map_id) + " - " + QString::fromUtf8 (name.c_str()));
    _selected_map->setItemData(count, QVariant (map_id));

    _corpse_map_id->addItem(QString::number(map_id) + " - " + QString::fromUtf8 (name.c_str()));
    _corpse_map_id->setItemData(count + 1, QVariant (map_id));

    count++;
  }

  auto add_btn = new QPushButton(this);
  add_btn->setIcon(noggit::ui::font_awesome_icon(noggit::ui::font_awesome::plus));
  layout_selector->addWidget(add_btn);

  auto remove_btn = new QPushButton(this);
  remove_btn->setIcon(noggit::ui::font_awesome_icon(noggit::ui::font_awesome::timescircle));
  layout_selector->addWidget(remove_btn);

  _map_settings = new QGroupBox("Map settings", this);
  layout_right->addWidget(_map_settings);

  auto map_settings_layout = new QFormLayout(_map_settings);
  _map_settings->setLayout(map_settings_layout);

  _directory = new QLineEdit(_map_settings);
  map_settings_layout->addRow("Map directory:", _directory);

  _instance_type = new QComboBox(_map_settings);
  _instance_type->addItem("None");
  _instance_type->setItemData(0, QVariant(0));

  _instance_type->addItem("Instance Crusade");
  _instance_type->setItemData(1, QVariant(1));

  _instance_type->addItem("Raid");
  _instance_type->setItemData(2, QVariant(2));

  _instance_type->addItem("Battleground");
  _instance_type->setItemData(3, QVariant(3));

  _instance_type->addItem("Arena");
  _instance_type->setItemData(4, QVariant(4));

  map_settings_layout->addRow("Map type:",_instance_type);

  _map_name = new LocaleDBCEntry(_map_settings);
  map_settings_layout->addRow("Map name:",_map_name);

  _area_table_id = new QSpinBox(_map_settings);
  map_settings_layout->addRow("Area ID:",_area_table_id);

  _map_desc_alliance = new LocaleDBCEntry(_map_settings);
  map_settings_layout->addRow("Description (Alliance):",_map_desc_alliance);

  _map_desc_horde = new LocaleDBCEntry(_map_settings);
  map_settings_layout->addRow("Description (Horde):",_map_desc_horde);

  _loading_screen  = new QSpinBox(_map_settings);
  map_settings_layout->addRow("Loading screen:",_loading_screen);

   _minimap_icon_scale = new QDoubleSpinBox(_map_settings);
  map_settings_layout->addRow("Minimap icon scale:",_minimap_icon_scale);

  _corpse_map_id->setCurrentText("None");
  map_settings_layout->addRow("Corpse map:",_corpse_map_id);

  _corpse_x = new QDoubleSpinBox(_map_settings);
  map_settings_layout->addRow("Corpse X:",_corpse_x);

  _corpse_y = new QDoubleSpinBox(_map_settings);
  map_settings_layout->addRow("Corpse Y:",_corpse_y);

  _time_of_day_override = new QSpinBox(_map_settings);
  _time_of_day_override->setMinimum(-1);
  _time_of_day_override->setValue(-1);

  map_settings_layout->addRow("Daytime override:",_time_of_day_override);

  _expansion_id = new QComboBox(_map_settings);

  _expansion_id->addItem("Classic");
  _expansion_id->setItemData(0, QVariant(0));

  _expansion_id->addItem("Burning Crusade");
  _expansion_id->setItemData(1, QVariant(1));

  _expansion_id->addItem("Wrath of the Lich King");
  _expansion_id->setItemData(2, QVariant(2));

  map_settings_layout->addRow("Expansion:",_expansion_id);

  _raid_offset = new QSpinBox(_map_settings);
  map_settings_layout->addRow("Raid offset:",_raid_offset);

  _max_players = new QSpinBox(_map_settings);
  map_settings_layout->addRow("Max players:",_max_players);

  selectMap(_selected_map->itemData(_selected_map->currentIndex()).toInt());

  // Connections

  connect(_selected_map, QOverload<int>::of(&QComboBox::currentIndexChanged)
      , [&] (int index)
            {
              selectMap(_selected_map->itemData(index).toInt());
            }
  );

}

void MapCreationWizard::selectMap(int map_id)
{

  DBCFile::Record record = gMapDB.getByID(map_id);

  _world = std::make_unique<World>(record.getString(MapDB::InternalName), map_id); // verify if leaks here
  _minimap_widget->world(_world.get());

  _directory->setText(record.getString(1));
  _instance_type->setCurrentIndex(record.getInt(2));

  _map_name->fill(record, 5);

  _area_table_id->setValue(record.getInt(22));

  _map_desc_alliance->fill(record, 23);
  _map_desc_horde->fill(record, 40);

  _loading_screen->setValue(record.getInt(57));
  _minimap_icon_scale->setValue(record.getFloat(58));

  int corpse_map_idx = record.getInt(59);
  for (int i = 0; i < _corpse_map_id->count(); ++i)
  {
    if (_corpse_map_id->itemData(i) == corpse_map_idx)
    {
      _corpse_map_id->setCurrentIndex(i);
    }
  }

  _corpse_x->setValue(record.getFloat(60));
  _corpse_y->setValue(record.getFloat(61));
  _time_of_day_override->setValue(record.getInt(62));
  _expansion_id->setCurrentIndex(record.getInt(63));
  _raid_offset->setValue(record.getInt(64));
  _max_players->setValue(record.getInt(65));
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
  // Connect

  connect ( _current_locale, &QComboBox::currentTextChanged
      , [&] (QString s)
            {
              setCurrentLocale(_current_locale->currentText().toStdString());
            }
  );
}

void LocaleDBCEntry::setCurrentLocale(const std::string& locale)
{
  _show_entry->setCurrentWidget(_widget_map.at(locale));
}

void LocaleDBCEntry::fill(DBCFile::Record& record, size_t field, size_t id_field)
{
  for (int loc = 0; loc < 16; ++loc)
  {
    setValue(record.getLocalizedString(field, loc), loc);
  }

  _flags->setValue(record.getInt(field + 16));
}



