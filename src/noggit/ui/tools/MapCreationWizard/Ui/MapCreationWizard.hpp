// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QStackedWidget>

#include <noggit/DBC.h>
#include <noggit/DBCFile.h>

#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/widget.hpp>
#include <noggit/World.h>


namespace Noggit
{

  namespace Ui::Tools::MapCreationWizard::Ui
  {

    class LocaleDBCEntry : public QWidget
    {
      public:
        LocaleDBCEntry(QWidget *parent = nullptr);

        void setCurrentLocale(const std::string& locale);

        void setValue(const std::string& val, int locale)
        {
          _widget_map.at(_locale_names[locale])->setText(QString::fromStdString(val));

          if (!val.empty() && _flags->value() == 0)
          {
              _flags->setValue(16712190); // default flags when there is text
          }
        }

        std::string getValue(int locale) { return  _widget_map.at(_locale_names[locale])->text().toStdString(); };

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
      void destroyFakeWorld() { if(_world) delete _world; _world = nullptr; _minimap_widget->world (nullptr); };
      void addNewMap();
    signals:
      void map_dbc_updated(int new_map = 0);

    private:
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

      World* _world = nullptr;

      bool _is_new_record = false;
      int _cur_map_id = -1;

      QMetaObject::Connection _connection;

      std::string getDifficultyString();

      void selectMap(int map_id);
      void selectMapDifficulty();

      void saveCurrentEntry();
      void discardChanges();

      void removeMap();

    };
  }
}
