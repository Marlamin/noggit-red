// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/DBC.h>
#include <noggit/ui/tools/MapCreationWizard/Ui/MapCreationWizard.hpp>

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox.h>
#include <QtWidgets/QComboBox.h>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListView>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QMediaPlayer>

#include <functional>
#include <string>

namespace Noggit
{
  namespace Ui
  {

    static std::map <int, std::string> area_flags_names = {
        {0 , "Emit Breath Particles"},
        {1 , "Breath Particles Override Parent"},
        {2 , "On Map Dungeon"},
        {3 , "Allow Trade Channel"},
        {4 , "Enemies PvP Flagged"},
        {5 , "Allow Resting"},
        {6 , "Allow Dueling"},
        {7 , "Free For All PvP"},
        {8 , "Linked Chat (Set in cities)"},
        {9 , "Linked Chat Special Area"},
        // {10, "Force this area when on a Dynamic Transport"}, // From Classic TBC definitions, probably chanegd around CATA
        {10, "Allow Flying"}, // in 3.3.5
        {11, "No PvP"},
        {12, "No Ghost on Release"},
        {13, "Sub-zone Ambient Multiplier"},
        {14, "Enable Flight Bounds on Map"}, // not confirmed in 3.3.5
        {15, "PVP POI"},
        {16, "No chat channels"},
        {17, "Area not in use"},
        {18, "Contested"},
        {19, "No Player Summoning"},
        {20, "No Dueling if Tournament Realm"},
        {21, "Players Call Guards"},
        {22, "Horde Resting"},
        {23, "Alliance Resting"},
        {24, "Combat Zone"},
        {25, "Force Indoors"},
        {26, "Force Outdoors"},
        {27, "Allow Hearth-and-Resurrect from Area"},
        // {28, "No Local Defense Channel"}, // From Classic TBC definitions, probably chanegd around CATA
        {28, "Flying not allowed"}, // in 3.3.5
        {29, "Only Evaluate Ghost Bind Once"},
        {30, "Is Subzone"},
        // {31, "Don't Evaluate Graveyard From Client"}
    };

    class AreaEditor : public QWidget
    {
        Q_OBJECT
    public:
        AreaEditor(QWidget* parent = nullptr);

        void load_area(int area_id);
    private:
        QLabel* _area_id_label;
        QLabel* _parent_area_label;
        int _parent_area_id = 0;
        QPushButton* _set_parent_button;
        QSpinBox* _flags_value_spinbox;
        Tools::MapCreationWizard::Ui::LocaleDBCEntry* _area_name;
        QCheckBox* flags_checkboxes[31]{ 0 };
        QSpinBox* _exploration_level_spinbox;
        QDoubleSpinBox* _min_elevation_spinbox;
        QSlider* _ambiant_multiplier;
        QComboBox* _faction_group_combobox;
        QComboBox* _sound_provider_preferences_cbbox;
        QComboBox* _underwater_sound_provider_preferences_cbbox;
        // QComboBox* _liquid_type_water_combobox;
        // QComboBox* _liquid_type_ocean_combobox;
        // QComboBox* _liquid_type_magma_combobox;
        // QComboBox* _liquid_type_slime_combobox;

        QPushButton* _zone_music_button;
        QPushButton* _zone_intro_music_button;
        QPushButton* _sound_ambiance_day_button;
        QPushButton* _sound_ambiance_night_button;

        void save_area();

        int _areabit = 0;
    };

    class zone_id_browser : public QWidget
    {
      Q_OBJECT

    public:
      zone_id_browser(QWidget* parent = nullptr);
      void setMapID(int id);
      void setZoneID(int id);
      void changeRadius(float change);
      void setRadius(float radius);

      float brushRadius() const { return _radius; }

      int GetSelectedAreaId();
      void buildAreaList();
      QTreeWidgetItem* create_or_get_tree_widget_item(int area_id);

    signals:
      void selected (int area_id);

    private:
      QTreeWidget* _area_tree;

      QSlider* _radius_slider;
      QDoubleSpinBox* _radius_spin;

      AreaEditor* _area_editor;

      std::map<int, QTreeWidgetItem*> _items;
      int mapID;
      float _radius = 15.0f;

      QTreeWidgetItem* add_area(int area_id);
      void open_area_editor();
      void add_new_zone();
      void add_new_subzone();
    };


  }
}
