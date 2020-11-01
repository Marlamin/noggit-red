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
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QStackedWidget>

#include <noggit/DBC.h>
#include <noggit/DBCFile.h>

#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/widget.hpp>

class World;

namespace noggit
{

  namespace Red::MapCreationWizard::Ui
  {

    class LocaleDBCEntry : public QWidget
    {
      public:
        LocaleDBCEntry(QWidget *parent = nullptr);

        void setCurrentLocale(const std::string& locale);

        void setValue(const std::string& val, int locale)
        {
          _widget_map.at(_locale_names[locale])->setText(QString::fromStdString(val));
        }

        void fill(DBCFile::Record& record, size_t field, size_t id_field = 0);

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

    class MapCreationWizard : public ui::widget
    {
    public:
      MapCreationWizard(QWidget *parent = nullptr);
      void wheelEvent(QWheelEvent *event) override;

    private:
      ui::minimap_widget* _minimap_widget;
      QComboBox* _selected_map;
      QGroupBox* _map_settings;

      // Map settings
      QLineEdit* _directory;
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

      std::unique_ptr<World> _world;

      void selectMap(int map_id);

    };
  }
}
