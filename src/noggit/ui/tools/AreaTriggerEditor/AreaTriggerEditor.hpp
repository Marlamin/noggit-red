// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/DBCFile.h>
#include <util/FlatMap.hpp>

#include <glm/vec3.hpp>

#include <QWidget>

#include <map>
#include <unordered_map>

class MapView;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QTabWidget;
class QVBoxLayout;
class QSpinBox;
class QDoubleSpinBox;

namespace Noggit
{
  struct area_trigger;

  namespace Ui
  {
    class Vector3fWidget;
  }
}

namespace Noggit::Ui::Tools
{
  class AreaTriggerEditor : public QWidget
  {
    Q_OBJECT
  public:
    AreaTriggerEditor(MapView* map_view, QWidget* parent = nullptr);

    void clearSelection();
    void set_selected(area_trigger& trigger);

    void selection_translated(glm::vec3& new_position);
    void selection_rotated(float new_rotation);
    void selection_scaled(glm::vec3& new_scale);
    void selection_scaled(float new_scale);

    void save();

    void deleteSelectedTrigger();

  Q_SIGNALS:
    void selectionChanged(uint32_t current_id);
    void areaTriggerCreated(uint32_t id, DBCFile::Record& record);

  private:
    struct areatrigger_description
    {
      uint32_t id = 0;
      std::string zone_name;
      std::string sub_category;
      std::string trigger_name;
      bool is_builtin = false;

      areatrigger_description() = default;
      areatrigger_description(uint32_t id, std::string zone_name, std::string sub_category, std::string trigger_name, bool is_builtin);
    };

    util::FlatMap<uint32_t, areatrigger_description> _trigger_descriptions;

    MapView* _map_view = nullptr;
    QListWidget* _list_widget = nullptr;

    uint32_t _max_dbc_entry = 0;

    struct
    {
      QSpinBox* entry_spinbox = nullptr;
      QSpinBox* map_spinbox = nullptr;
      QLineEdit* zone_name_widget = nullptr;
      QLineEdit* category_name_widget = nullptr;
      QLineEdit* trigger_name_widget = nullptr;
      Vector3fWidget* position_widget = nullptr;
      Vector3fWidget* size_widget = nullptr;
      QDoubleSpinBox* rotation_widget = nullptr;
      QDoubleSpinBox* radius_widget = nullptr;
    } _selection_widgets;

    std::unordered_map<uint32_t, QListWidgetItem*> _list_items;

    uint32_t _selected_trigger_id = std::numeric_limits<uint32_t>::max();

    void parseDescriptions();

    std::string formatTriggerName(uint32_t trigger_id);

    void createListView(QVBoxLayout* layout);
    void createSelectionWidgets(QVBoxLayout* layout);
    void createInfoWidgets(QVBoxLayout* layout);

    enum class TriggerKind { Sphere, Box };
    void addNewTrigger(TriggerKind kind);
  };
}
