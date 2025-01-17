// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/DBCFile.h>
#include <noggit/MapHeaders.h>
#include <noggit/ui/widget.hpp>

#include <QWidget>

#include <memory>
#include <unordered_map>
#include <vector>

namespace BlizzardDatabaseLib::Structures
{
  struct BlizzardDatabaseRow;
}

class WMOInstance;
class World;

class QLineEdit;
class QDoubleSpinBox;
class QComboBox;
class QGroupBox;
class QSpinBox;
class QCheckBox;
class QStackedWidget;
class QTabWidget;

namespace Noggit
{
  namespace Project
  {
    class NoggitProject;
  }

  namespace Ui
  {
    class Vector3fWidget;
    class minimap_widget;
  }

  namespace Ui::Tools::MapCreationWizard::Ui
  {

    class LocaleDBCEntry : public QWidget
    {
      public:
        LocaleDBCEntry(QWidget *parent = nullptr);

        void setCurrentLocale(const std::string& locale);

        void setValue(const std::string& val, int locale);

        std::string getValue(int locale);;

        void fill(DBCFile::Record& record, size_t field);
        void fill(BlizzardDatabaseLib::Structures::BlizzardDatabaseRow& record, std::string columnName);
        void clear();
        void toRecord(DBCFile::Record& record, size_t field);
        void setDefaultLocValue(const std::string& text);

      private:
        QComboBox* _current_locale;
        QStackedWidget* _show_entry;

        std::unordered_map<std::string, QLineEdit*> _widget_map;
        std::vector<std::string> _locale_names = {"enUS", "koKR", "frFR", "deDE", "zhCN",
                                                  "zhTW", "esES", "esMX", "ruRU", "jaJP", "ptPT", "itIT",
                                                  "Unknown 1", "Unknown 2", "Unknown 3", "Unknown 4"};

        QLineEdit* _en;
        QLineEdit* _kr;
        QLineEdit* _fr;
        QLineEdit* _de;
        QLineEdit* _cn; // + nCN
        QLineEdit* _tw;
        QLineEdit* _es;
        QLineEdit* _mx;
        QLineEdit* _ru;
        QLineEdit* _jp;
        QLineEdit* _pt;
        QLineEdit* _it;
        QLineEdit* _unk1;
        QLineEdit* _unk2;
        QLineEdit* _unk3;
        QLineEdit* _unk4;
        QSpinBox* _flags;
    };

  class MapCreationWizard : public Noggit::Ui::widget
    {
      Q_OBJECT

    public:
      MapCreationWizard(std::shared_ptr<Project::NoggitProject> project, QWidget *parent = nullptr);
      ~MapCreationWizard();

      void wheelEvent(QWheelEvent *event) override;
      // void destroyFakeWorld() { if(_world) _world.reset(); _world = nullptr; _minimap_widget->world (nullptr); };
      void addNewMap();

      World* getWorld() const;;
      std::unique_ptr<World> _world;

    signals:
      void map_dbc_updated(int new_map = 0);

    private:
      struct WmoEntryTab
      {
          QCheckBox* disableTerrain = nullptr;
          QLineEdit* wmoPath = nullptr;
          QSpinBox* nameId = nullptr;
          QSpinBox* uniqueId = nullptr;
          Vector3fWidget* position = nullptr;
          Vector3fWidget* rotation = nullptr;
          QSpinBox* flags = nullptr;
          QComboBox* doodadSet = nullptr;
          QComboBox* nameSet = nullptr;

          ENTRY_MODF wmoEntry;
      };

      std::shared_ptr<Project::NoggitProject> _project;
      Noggit::Ui::minimap_widget* _minimap_widget;
      int _selected_map;
      QGroupBox* _map_settings;

      QTabWidget* _tabs;

      // Map settings
      QLineEdit* _directory;

      QCheckBox* _is_big_alpha;
      QCheckBox* _sort_by_size_cat;

      QComboBox* _instance_type;

      LocaleDBCEntry* _map_name;

      QSpinBox* _area_table_id;

      LocaleDBCEntry* _map_desc_alliance;
      LocaleDBCEntry* _map_desc_horde;

      QSpinBox* _loading_screen;
      QDoubleSpinBox* _minimap_icon_scale;
      QComboBox *_corpse_map_id;

      QDoubleSpinBox* _corpse_x;
      QDoubleSpinBox* _corpse_y;

      QSpinBox* _time_of_day_override;
      QComboBox* _expansion_id;
      QSpinBox* _raid_offset;
      QSpinBox* _max_players;

      // map difficulty settings
      QComboBox* _difficulty_type;
      LocaleDBCEntry* _difficulty_req_message;
      QSpinBox* _difficulty_raid_duration;
      QSpinBox* _difficulty_max_players;
      QLineEdit* _difficulty_string;

      WmoEntryTab _wmoEntryTab;

      bool _is_new_record = false;
      int _cur_map_id = -1;

      QMetaObject::Connection _connection;

      std::string getDifficultyString();

      void selectMap(int map_id);
      void selectMapDifficulty();

      void saveCurrentEntry();
      void discardChanges();

      void removeMap();

      void createMapSettingsTab();
      void createDifficultyTab();
      void createWmoEntryTab();

      void populateWmoEntryTab();
      void populateDoodadSet(WMOInstance& instance);
      void populateNameSet(WMOInstance& instance);
    };
  }
}
