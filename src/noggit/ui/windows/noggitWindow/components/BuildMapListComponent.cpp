// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/application/Utils.hpp>
#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/windows/noggitWindow/components/BuildMapListComponent.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include <noggit/ui/windows/noggitWindow/widgets/MapListItem.hpp>
#include <noggit/World.h>

#include <blizzard-database-library/include/BlizzardDatabase.h>

#include <QAction>
#include <QListWidgetItem>
#include <QMenu>
#include <QObject>

using namespace Noggit::Ui::Component;

void BuildMapListComponent::buildMapList(Noggit::Ui::Windows::NoggitWindow* parent)
{
  {
    QSignalBlocker const blocker(parent->_continents_table);
    parent->_continents_table->clear(); // // calls itemSelectionChanged
  }

  const auto& table = std::string("Map");
  auto map_table = parent->_project->ClientDatabase->LoadTable(table, readFileAsIMemStream);

  auto iterator = map_table.Records();
  auto pinned_maps = std::vector<Widget::MapListData>();
  auto maps = std::vector<Widget::MapListData>();
  while (iterator.HasRecords())
  {
    auto record = iterator.Next();

    Widget::MapListData map_list_data{};

    for (auto const& value: record.Columns["MapName_lang"].Values)
    {
      if (value.empty())
        continue;

      map_list_data.map_name = QString::fromUtf8(value.c_str());
      break;
    }

    map_list_data.map_id = record.RecordId;
    map_list_data.map_type_id = std::stoi(record.Columns["InstanceType"].Value);
    map_list_data.expansion_id = std::stoi(record.Columns["ExpansionID"].Value);

    if (map_list_data.map_type_id < 0 || map_list_data.map_type_id > 5 || !World::IsEditableWorld(record))
      continue;

    map_list_data.wmo_map = (World::IsWMOWorld(record));

    auto project_pinned_maps = parent->_project->PinnedMaps;

    auto pinned_map_found = std::find_if(std::begin(project_pinned_maps), std::end(project_pinned_maps),
                                         [&](Project::NoggitProjectPinnedMap pinned_map)
                                       {
                                         return pinned_map.MapId == map_list_data.map_id;
                                       });

    if (pinned_map_found != std::end(project_pinned_maps))
    {
      map_list_data.pinned = true;
      pinned_maps.push_back(map_list_data);
    }
    else
    {
      maps.push_back(map_list_data);
    }
  }

  pinned_maps.insert(pinned_maps.end(), maps.begin(), maps.end());

  for (auto const& map: pinned_maps)
  {
    auto map_list_item = new Widget::MapListItem(map, parent->_continents_table);
    auto item = new QListWidgetItem(parent->_continents_table);

    if (map.pinned)
    {
      QObject::connect(map_list_item, &QListWidget::customContextMenuRequested,
                       [=](const QPoint& pos)
                       {
                         QMenu context_menu(map_list_item->tr("Context menu"), map_list_item);

                         QAction action_1("Unpin Map", map_list_item);
                         auto icon = QIcon();
                         icon.addPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16, 16)));
                         action_1.setIcon(icon);

                         QObject::connect(&action_1, &QAction::triggered, [=]()
                         {
                           parent->handleEventMapListContextMenuUnpinMap(map.map_id);
                         });

                         context_menu.addAction(&action_1);
                         context_menu.exec(map_list_item->mapToGlobal(pos));
                       });
    }
    else
    {
      QObject::connect(map_list_item, &QListWidget::customContextMenuRequested,
                       [=](const QPoint& pos)
                       {
                         QMenu context_menu(map_list_item->tr("Context menu"), map_list_item);
                         QAction action_1("Pin Map", map_list_item);
                         auto icon = QIcon();
                         icon.addPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16, 16)));
                         action_1.setIcon(icon);

                         QObject::connect(&action_1, &QAction::triggered, [=]()
                         {
                           parent->handleEventMapListContextMenuPinMap(map.map_id, map.map_name.toStdString());
                         });

                         context_menu.addAction(&action_1);
                         context_menu.exec(map_list_item->mapToGlobal(pos));
                       });
    }

    item->setSizeHint(map_list_item->minimumSizeHint());
    item->setData(Qt::UserRole, QVariant(map.map_id));
    parent->_continents_table->setItemWidget(item, map_list_item);
  }

  parent->_project->ClientDatabase->UnloadTable(table);
}
