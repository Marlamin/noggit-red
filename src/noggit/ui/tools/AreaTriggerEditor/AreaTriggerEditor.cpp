// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "AreaTriggerEditor.hpp"

#include <noggit/application/NoggitApplication.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/area_trigger.hpp>
#include <noggit/StringHash.hpp>
#include <noggit/MapView.h>
#include <noggit/ui/FontNoggit.hpp>
#include <noggit/ui/widgets/Vector3Widget.hpp>

#include <rapidfuzz/fuzz.hpp>

#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDialog>

#include <concepts>
#include <format>
#include <fstream>
#include <functional>
#include <string_view>
#include <system_error>
#include <tuple>
#include <vector>

namespace Noggit::Ui::Tools
{
  constexpr int TRIGGER_ID_USERDATA = Qt::UserRole + 1;

  AreaTriggerEditor::areatrigger_description::areatrigger_description(
    uint32_t id,
    std::string zone_name,
    std::string sub_category,
    std::string trigger_name,
    bool is_builtin)
    : id{ id }
    , zone_name{ std::move(zone_name) }
    , sub_category{ std::move(sub_category) }
    , trigger_name{ std::move(trigger_name) }
    , is_builtin{ is_builtin }
  {
  }

  AreaTriggerEditor::AreaTriggerEditor(MapView* map_view, QWidget* parent)
    : QWidget{ parent }
    , _map_view{ map_view }
  {
    parseDescriptions();

    setMinimumWidth(250);

    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);

    createListView(layout);
    createSelectionWidgets(layout);
    createInfoWidgets(layout);

    for (auto&& record : gAreaTriggerDB)
    {
      auto const id = record.getUInt(AreaTriggerDB::Id);
      _max_dbc_entry = std::max(_max_dbc_entry, id);
    }
  }

  void AreaTriggerEditor::clearSelection()
  {
    _selected_trigger_id = std::numeric_limits<uint32_t>::max();

    _list_widget->clearSelection();

    _selection_widgets.zone_name_widget->clear();
    _selection_widgets.category_name_widget->clear();
    _selection_widgets.trigger_name_widget->clear();

    _selection_widgets.entry_spinbox->clear();
    _selection_widgets.map_spinbox->clear();
    _selection_widgets.position_widget->clear();
    _selection_widgets.size_widget->clear();
    _selection_widgets.rotation_widget->clear();
    _selection_widgets.radius_widget->clear();
  }

  void AreaTriggerEditor::set_selected(area_trigger& trigger)
  {
    _selected_trigger_id = trigger.id;

    if (auto itr = _list_items.find(trigger.id); itr != _list_items.end())
    {
      QSignalBlocker const _a(_list_widget);
      itr->second->setSelected(true);
      _list_widget->scrollToItem(itr->second);
    }

    if (auto description = _trigger_descriptions.At(_selected_trigger_id); description)
    {
      _selection_widgets.zone_name_widget->setText(description->zone_name.c_str());
      _selection_widgets.category_name_widget->setText(description->sub_category.c_str());
      _selection_widgets.trigger_name_widget->setText(description->trigger_name.c_str());
    }

    QSignalBlocker const _a(_selection_widgets.entry_spinbox);
    QSignalBlocker const _b(_selection_widgets.map_spinbox);
    QSignalBlocker const _c(_selection_widgets.position_widget);
    QSignalBlocker const _d(_selection_widgets.radius_widget);
    QSignalBlocker const _e(_selection_widgets.size_widget);
    QSignalBlocker const _f(_selection_widgets.rotation_widget);

    _selection_widgets.entry_spinbox->setValue(trigger.id);
    _selection_widgets.map_spinbox->setValue(trigger.map_id);
    _selection_widgets.position_widget->setValue(&trigger.position.x);

    std::visit([&](auto&& t) {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(t)>, sphere_trigger>)
      {
        _selection_widgets.radius_widget->setValue(t.radius);

        _selection_widgets.radius_widget->setDisabled(false);
        _selection_widgets.size_widget->setDisabled(true);
        _selection_widgets.rotation_widget->setDisabled(true);
        _selection_widgets.size_widget->clear();
        _selection_widgets.rotation_widget->clear();
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(t)>, box_trigger>)
      {
        auto size = t.extents_max - t.extents_min;
        _selection_widgets.size_widget->setValue(&size.x);
        _selection_widgets.rotation_widget->setValue(t.orientation);

        _selection_widgets.radius_widget->setDisabled(true);
        _selection_widgets.radius_widget->clear();
        _selection_widgets.size_widget->setDisabled(false);
        _selection_widgets.rotation_widget->setDisabled(false);
      }
      }, trigger.trigger);

    emit selectionChanged(trigger.id);
  }

  void AreaTriggerEditor::selection_translated(glm::vec3& new_position)
  {
    QSignalBlocker const _(_selection_widgets.position_widget);
    _selection_widgets.position_widget->setValue(&new_position.x);
  }

  void AreaTriggerEditor::selection_rotated(float new_rotation)
  {
    QSignalBlocker const _(_selection_widgets.rotation_widget);
    _selection_widgets.rotation_widget->setValue(new_rotation);
  }

  void AreaTriggerEditor::selection_scaled(glm::vec3& new_scale)
  {
    QSignalBlocker const _(_selection_widgets.size_widget);
    _selection_widgets.size_widget->setValue(&new_scale.x);
  }

  void AreaTriggerEditor::selection_scaled(float new_scale)
  {
    QSignalBlocker const _(_selection_widgets.radius_widget);
    _selection_widgets.radius_widget->setValue(new_scale);
  }

  void AreaTriggerEditor::save()
  {
    auto const file_path = Noggit::Application::NoggitApplication::instance()->getConfiguration()->ApplicationNoggitDefinitionsPath
      + "\\AreatriggerDescriptions.csv";

    std::ofstream file{ file_path, std::ios_base::out };
    if (!file)
    {
      throw std::exception{ std::format("Could not open file {}!", file_path).c_str() };
    }

    file << "ID,Zone Name,Sub Category,Trigger Name,IsBuiltIn,\n";

    for (auto&& [_, desc] : _trigger_descriptions)
    {
      file << std::format("{0},{1},{2},{3},{4},\n", desc.id, desc.zone_name, desc.sub_category, desc.trigger_name, desc.is_builtin ? '1' : '0');
    }
  }

  void AreaTriggerEditor::parseDescriptions()
  {
    constexpr int expected_num_tokens = 6;
    char const* expeceted_header = "ID,Zone Name,Sub Category,Trigger Name,IsBuiltIn,";

    auto const file_path = Noggit::Application::NoggitApplication::instance()->getConfiguration()->ApplicationNoggitDefinitionsPath
      + "\\AreatriggerDescriptions.csv";

    QFile file{ QString::fromStdString(file_path) };
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      throw std::exception{ std::format("Could not open file {}!", file_path).c_str() };
    }

    QTextStream stream{ &file };
    if (auto header = stream.readLine(); header != expeceted_header)
    {
      auto foo = header.toStdString();
      throw std::exception{ std::format("File {} uses invalid header `{}`!", file_path, header.toStdString()).c_str() };
    }

    int line = 1;
    std::vector<std::pair<uint32_t, areatrigger_description>> descriptions;
    while (!stream.atEnd())
    {
      ++line;
      auto tokens = stream.readLine().split(',');
      if (tokens.size() != expected_num_tokens)
      {
        LogError << std::format("Encountered wrong number of tokens (expected: {}, encountered: {}) in file {}!", expected_num_tokens, tokens.size(), file_path);
        continue;
      }

      descriptions.emplace_back(std::make_pair(tokens[0].toUInt()
        , areatrigger_description{
          tokens[0].toUInt(),
          tokens[1].trimmed().toStdString(),
          tokens[2].trimmed().toStdString(),
          tokens[3].trimmed().toStdString(),
          tokens[4].toUInt() == 1
        }));
    }

    _trigger_descriptions = util::FlatMap<uint32_t, areatrigger_description>{ std::move(descriptions) };
  }

  std::string AreaTriggerEditor::formatTriggerName(uint32_t trigger_id)
  {
    auto info = _trigger_descriptions.At(trigger_id);
    if (!info)
    {
      return std::format("{}: UNKNOWN", trigger_id);
    }

    return std::format("{}: {} - {} - {}", trigger_id, info->zone_name, info->sub_category, info->trigger_name);
  }

  void AreaTriggerEditor::createListView(QVBoxLayout* layout)
  {
    auto search_layout = new QHBoxLayout(this);
    layout->addLayout(search_layout);

    auto search_bar = new QLineEdit{ this };
    search_layout->addWidget(search_bar);

    auto search_btn = new QPushButton{ this };
    search_btn->setMaximumWidth(20);
    search_btn->setHidden(true);
    search_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::times));
    search_layout->addWidget(search_btn, 0, Qt::AlignLeft);

    connect(search_btn, &QPushButton::pressed, [search_bar, search_btn] {
      search_bar->clear();
      search_btn->setHidden(true);
      });

    connect(search_bar, &QLineEdit::textChanged, [&, search_btn](QString const& new_text) {
      constexpr double score_cutoff = 50.0;

      if (new_text.isEmpty())
      {
        search_btn->setHidden(true);
        for (auto&& [_, item] : _list_items)
        {
          item->setHidden(false);
        }
      }
      else
      {
        search_btn->setHidden(false);
        for (auto&& [_, item] : _list_items)
        {
          bool hide = rapidfuzz::fuzz::token_set_ratio(item->text().toStdString(), new_text.toStdString()) <= score_cutoff;
          item->setHidden(hide);
        }
      }
      });

    _list_widget = new QListWidget(this);
    auto list_layout = new QVBoxLayout(_list_widget);
    list_layout->setContentsMargins(0, 0, 0, 0);

    _list_widget->setViewMode(QListView::ListMode);
    _list_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    _list_widget->setSelectionBehavior(QAbstractItemView::SelectItems);
    _list_widget->setUniformItemSizes(true);

    for (auto&& record : gAreaTriggerDB)
    {
      area_trigger trigger{ record };
      if (trigger.map_id != _map_view->getWorld()->getMapID())
      {
        continue;
      }

      auto* item = new QListWidgetItem();
      item->setData(TRIGGER_ID_USERDATA, QVariant{ trigger.id });

      if (trigger.trigger.index() == 0)
      {
        item->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::AREA_TRIGGER_SPHERE));
      }
      else
      {
        item->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::AREA_TRIGGER));
      }
      item->setText(formatTriggerName(trigger.id).c_str());
      _list_widget->addItem(item);

      _list_items[trigger.id] = item;
    }

    connect(_list_widget, &QListWidget::currentItemChanged, [=](QListWidgetItem* current, QListWidgetItem* previous) {
      uint32_t previous_id = std::numeric_limits<uint32_t>::max();
      if (previous)
      {
        previous_id = previous->data(TRIGGER_ID_USERDATA).toUInt();
      }
      QSignalBlocker _{ _list_widget };
      area_trigger trigger{ current->data(TRIGGER_ID_USERDATA).toUInt() };
      set_selected(trigger);
      });

    layout->addWidget(_list_widget);

    auto btn_layout = new QHBoxLayout(this);
    layout->addLayout(btn_layout);

    auto add_btn = new QPushButton("Add New", this);
    add_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
    connect(add_btn, &QPushButton::pressed, [=] {
      auto dialog = new QDialog{ this };
      dialog->setWindowFlag(Qt::Dialog);
      dialog->setWindowTitle("Add new Area Trigger...");
      dialog->setFixedWidth(250);
      auto layout = new QVBoxLayout{ dialog };

      auto radio_layout = new QHBoxLayout{ dialog };
      layout->addLayout(radio_layout);

      auto radio1 = new QRadioButton{ "Sphere" };
      radio1->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::AREA_TRIGGER_SPHERE));
      radio1->setChecked(true);
      radio_layout->addWidget(radio1);

      auto radio2 = new QRadioButton{ "Box" };
      radio2->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::AREA_TRIGGER));
      radio_layout->addWidget(radio2);

      auto btn_layout = new QHBoxLayout{ dialog };
      layout->addLayout(btn_layout);

      auto add_btn = new QPushButton{ "Add", dialog };
      connect(add_btn, &QPushButton::pressed, [=] {
        addNewTrigger(radio1->isChecked() ? TriggerKind::Sphere : TriggerKind::Box);
        dialog->close();
        });
      btn_layout->addWidget(add_btn);

      auto cancel_btn = new QPushButton{ "Cancel", dialog };
      connect(cancel_btn, &QPushButton::pressed, [=] { dialog->close(); });
      btn_layout->addWidget(cancel_btn);
      dialog->show();
      });
    btn_layout->addWidget(add_btn);

    auto remove_btn = new QPushButton("Remove Selected", this);
    remove_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::times));
    connect(remove_btn, &QPushButton::pressed, [=] {
      deleteSelectedTrigger();
      });
    btn_layout->addWidget(remove_btn);
  }

  void AreaTriggerEditor::createSelectionWidgets(QVBoxLayout* layout)
  {
    auto group_box = new QGroupBox("Selected");
    auto radius_layout = new QFormLayout(group_box);

    auto update_text = [&]()
      {
        if (_selected_trigger_id == std::numeric_limits<uint32_t>::max())
        {
          return;
        }

        _trigger_descriptions.Transform(_selected_trigger_id, [&](uint32_t key, areatrigger_description& description)
          {
            description.zone_name = _selection_widgets.zone_name_widget->text().toStdString();
            description.sub_category = _selection_widgets.category_name_widget->text().toStdString();
            description.trigger_name = _selection_widgets.trigger_name_widget->text().toStdString();
          });

        _list_items[_selected_trigger_id]->setText(QString("%1: %2 - %3 - %4").arg(_selected_trigger_id)
          .arg(_selection_widgets.zone_name_widget->text()
            , _selection_widgets.category_name_widget->text()
            , _selection_widgets.trigger_name_widget->text()));
      };

    _selection_widgets.zone_name_widget = new QLineEdit{ group_box };
    connect(_selection_widgets.zone_name_widget, &QLineEdit::textEdited, update_text);
    radius_layout->addRow("Zone", _selection_widgets.zone_name_widget);

    _selection_widgets.category_name_widget = new QLineEdit{ group_box };
    connect(_selection_widgets.category_name_widget, &QLineEdit::textEdited, update_text);
    radius_layout->addRow("Category", _selection_widgets.category_name_widget);

    _selection_widgets.trigger_name_widget = new QLineEdit{ group_box };
    connect(_selection_widgets.trigger_name_widget, &QLineEdit::textEdited, update_text);
    radius_layout->addRow("Name", _selection_widgets.trigger_name_widget);

    _selection_widgets.entry_spinbox = new QSpinBox{ group_box };
    _selection_widgets.entry_spinbox->setReadOnly(true);
    _selection_widgets.entry_spinbox->setMinimum(0);
    _selection_widgets.entry_spinbox->setMaximum(std::numeric_limits<int>::max());
    radius_layout->addRow("ID", _selection_widgets.entry_spinbox);

    _selection_widgets.map_spinbox = new QSpinBox{ group_box };
    _selection_widgets.map_spinbox->setReadOnly(true);
    _selection_widgets.map_spinbox->setMinimum(0);
    _selection_widgets.map_spinbox->setMaximum(std::numeric_limits<int>::max());
    radius_layout->addRow("Map-ID", _selection_widgets.map_spinbox);

    _selection_widgets.position_widget = new Vector3fWidget{ group_box };
    connect(_selection_widgets.position_widget, &Vector3fWidget::valueChanged, [=](glm::vec3 const& value) {
      if (_selected_trigger_id == std::numeric_limits<uint32_t>::max())
      {
        return;
      }

      area_trigger trigger{ _selected_trigger_id };

      NOGGIT_ACTION_MGR->beginAction(_map_view, Noggit::ActionFlags::eAREA_TRIGGER_TRANSFORMED, Noggit::ActionModalityControllers::eLMB);
      NOGGIT_CUR_ACTION->registerAreaTriggerTransformed(&trigger);

      trigger.position = value;
      trigger.write_to_dbc();
      });
    radius_layout->addRow("Position", _selection_widgets.position_widget);

    _selection_widgets.size_widget = new Vector3fWidget{ group_box };
    connect(_selection_widgets.size_widget, &Vector3fWidget::valueChanged, [=](glm::vec3 const& value) {
      if (_selected_trigger_id == std::numeric_limits<uint32_t>::max())
      {
        return;
      }

      area_trigger trigger{ _selected_trigger_id };

      NOGGIT_ACTION_MGR->beginAction(_map_view, Noggit::ActionFlags::eAREA_TRIGGER_TRANSFORMED, Noggit::ActionModalityControllers::eLMB);
      NOGGIT_CUR_ACTION->registerAreaTriggerTransformed(&trigger);

      if (trigger.trigger.index() != 1)
      {
        return;
      }

      auto&& box = std::get<1>(trigger.trigger);
      box.extents_min = glm::vec3{ -value.x / 2, -value.y / 2, -value.z / 2 };
      box.extents_max = glm::vec3{ value.x / 2,  value.y / 2,  value.z / 2 };
      trigger.write_to_dbc();
      });
    radius_layout->addRow("Size", _selection_widgets.size_widget);

    _selection_widgets.rotation_widget = new QDoubleSpinBox{ group_box };
    connect(_selection_widgets.rotation_widget, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
      if (_selected_trigger_id == std::numeric_limits<uint32_t>::max())
      {
        return;
      }

      area_trigger trigger{ _selected_trigger_id };

      NOGGIT_ACTION_MGR->beginAction(_map_view, Noggit::ActionFlags::eAREA_TRIGGER_TRANSFORMED, Noggit::ActionModalityControllers::eLMB);
      NOGGIT_CUR_ACTION->registerAreaTriggerTransformed(&trigger);

      if (trigger.trigger.index() != 1)
      {
        return;
      }

      std::get<1>(trigger.trigger).orientation = static_cast<float>(value);
      trigger.write_to_dbc();
      });
    radius_layout->addRow("Rotation", _selection_widgets.rotation_widget);

    _selection_widgets.radius_widget = new QDoubleSpinBox{ group_box };
    _selection_widgets.radius_widget->setMinimum(0.001);
    _selection_widgets.radius_widget->setMaximum(500);
    connect(_selection_widgets.radius_widget, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
      if (_selected_trigger_id == std::numeric_limits<uint32_t>::max())
      {
        return;
      }

      area_trigger trigger{ _selected_trigger_id };

      NOGGIT_ACTION_MGR->beginAction(_map_view, Noggit::ActionFlags::eAREA_TRIGGER_TRANSFORMED, Noggit::ActionModalityControllers::eLMB);
      NOGGIT_CUR_ACTION->registerAreaTriggerTransformed(&trigger);

      if (trigger.trigger.index() != 0)
      {
        return;
      }

      std::get<0>(trigger.trigger).radius = static_cast<float>(value);
      trigger.write_to_dbc();
      });
    radius_layout->addRow("Radius", _selection_widgets.radius_widget);

    layout->addWidget(group_box);
  }

  void generate_hotkey_row(std::initializer_list<FontNoggit::Icons>&& hotkeys, const char* description, QWidget* parent, QFormLayout* layout)
  {
    auto row_layout = new QHBoxLayout(parent);

    const char* from = nullptr;
    auto icon = hotkeys.begin();

    while (*description)
    {
      if (*description == '\a')
      {
        if (from)
        {
          auto label = new QLabel(::std::string(from, description - from).c_str());
          row_layout->addWidget(label);
        }

        auto label = new QLabel(parent);
        QIcon hotkey_icon = FontNoggitIcon(*icon++);
        label->setPixmap(hotkey_icon.pixmap(22, 22));
        row_layout->addWidget(label);

        from = ++description;
      }
      else
      {
        if (!from)
        {
          from = description;
        }
        ++description;
      }
    }

    if (from && *from)
    {
      auto label = new QLabel(from);
      row_layout->addWidget(label);
    }
    row_layout->setAlignment(Qt::AlignLeft);
    layout->addRow(row_layout);
  }

  void AreaTriggerEditor::createInfoWidgets(QVBoxLayout* layout)
  {
    auto group_box = new QGroupBox("Info");
    auto radius_layout = new QFormLayout(group_box);
    layout->addWidget(group_box);

    generate_hotkey_row({ FontNoggit::mmb }, "\aJump to selected Trigger", group_box, radius_layout);
  }

  void AreaTriggerEditor::addNewTrigger(TriggerKind kind)
  {
    auto&& record = gAreaTriggerDB.addRecord(++_max_dbc_entry);
    area_trigger trigger{ record };
    trigger.id = _max_dbc_entry;
    trigger.map_id = _map_view->getWorld()->getMapID();
    trigger.position = _map_view->getCamera()->position;

    auto selection = _map_view->intersect_result(false);
    if (!selection.empty())
    {
      std::visit([&](auto&& selection) {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(selection)>, selected_object_type>)
        {
          // TODO: get the hit vertex and create the area trigger there
        }
        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(selection)>, selected_chunk_type>)
        {
          trigger.position = selection.position;
        }

        }, selection.front().second);
    }

    QIcon icon = Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::AREA_TRIGGER_SPHERE);
    if (kind == TriggerKind::Sphere)
    {
      trigger.trigger = sphere_trigger{ .radius = 1.f };
    }
    else
    {
      trigger.trigger = box_trigger
      {
        .extents_min = { -0.5f, 0.0f, -0.5f },
        .extents_max = { 0.5f, 1.0f, 0.5f },
        .orientation = 0.f,
      };
      icon = Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::AREA_TRIGGER);
    }
    trigger.write_to_dbc();
    emit areaTriggerCreated(trigger.id, record);

    areatrigger_description desc{ trigger.id, "ZoneName", "SubCategory", "TriggerName", false };
    _trigger_descriptions.Insert(trigger.id, desc);

    auto* item = new QListWidgetItem();
    item->setData(TRIGGER_ID_USERDATA, QVariant{ trigger.id });
    item->setIcon(icon);
    item->setText(formatTriggerName(trigger.id).c_str());
    _list_widget->addItem(item);
    _list_items[trigger.id] = item;

    set_selected(trigger);
  }

  void AreaTriggerEditor::deleteSelectedTrigger()
  {
    if (_selected_trigger_id == std::numeric_limits<uint32_t>::max())
    {
      return;
    }

    if (_selected_trigger_id == std::numeric_limits<uint32_t>::max())
    {
      return;
    }

    auto dialog = new QDialog{ this };
    dialog->setWindowFlag(Qt::Dialog);
    dialog->setWindowTitle("Remove Area Trigger...");
    dialog->setFixedWidth(250);
    auto layout = new QVBoxLayout{ dialog };

    auto label = new QLabel{ "Are you sure you want to delete the selected\nArea Trigger?", dialog };
    layout->addWidget(label);

    if (auto desc = _trigger_descriptions.At(_selected_trigger_id); desc)
    {
      if (desc->is_builtin)
      {
        auto label2 = new QLabel{ "THIS WILL DELETE A BUILT-IN TRIGGER!\n"
          "THIS MAY BREAK THE GAME!\n\n"
          "Don't do this if you don't know what you're\n"
          "doing!", dialog };
        label2->setStyleSheet("QLabel { color: RED }");
        layout->addWidget(label2);
      }
    }

    auto btn_layout = new QHBoxLayout{ dialog };
    layout->addLayout(btn_layout);

    auto yes_btn = new QPushButton{ "Yes", dialog };
    connect(yes_btn, &QPushButton::pressed, [=] {
      _list_items.erase(_selected_trigger_id);

      _trigger_descriptions.Erase(_selected_trigger_id);

      gAreaTriggerDB.removeRecord(_selected_trigger_id);

      clearSelection();
      emit selectionChanged(_selected_trigger_id);

      dialog->close();
      });
    btn_layout->addWidget(yes_btn);

    auto no_btn = new QPushButton{ "No", dialog };
    connect(no_btn, &QPushButton::pressed, [=] {
      dialog->close();
      });
    btn_layout->addWidget(no_btn);

    dialog->show();
  }
}
