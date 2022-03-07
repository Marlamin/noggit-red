// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/windows/noggitWindow/components/BuildMapListComponent.hpp>
#include <noggit/ui/windows/noggitWindow/widgets/MapListItem.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include <noggit/application/Utils.hpp>
#include <QMenuBar>
#include <QAction>
#include <QObject>

using namespace Noggit::Ui::Component;

void BuildMapListComponent::BuildMapList(Noggit::Ui::Windows::NoggitWindow* parent)
{
  parent->_continents_table->clear();

  const auto& table = std::string("Map");
  auto mapTable = parent->_project->ClientDatabase->LoadTable(table, readFileAsIMemStream);

  auto iterator = mapTable.Records();
  auto pinnedMaps = std::vector<Widget::MapListData>();
  auto maps = std::vector<Widget::MapListData>();
  while (iterator.HasRecords())
  {
    auto record = iterator.Next();

    auto mapListData = Widget::MapListData();

    for (auto const& value : record.Columns["MapName_lang"].Values)
    {
      if (value.empty())
        continue;

      mapListData.MapName = QString::fromUtf8(value.c_str());
      break;
    }

    mapListData.MapId = record.RecordId;
    mapListData.MapTypeId = std::stoi(record.Columns["InstanceType"].Value);
    mapListData.ExpansionId = std::stoi(record.Columns["ExpansionID"].Value);

    if (mapListData.MapTypeId < 0 || mapListData.MapTypeId > 5 || !World::IsEditableWorld(record))
      continue;

    auto projectPinnedMaps = parent->_project->PinnedMaps;

    auto pinnedMapFound = std::find_if(std::begin(projectPinnedMaps), std::end(projectPinnedMaps), [&](Project::NoggitProjectPinnedMap pinnedMap)
    {
    	return pinnedMap.MapId == mapListData.MapId;
    });

    if (pinnedMapFound != std::end(projectPinnedMaps))
    {
    	mapListData.Pinned = true;
      pinnedMaps.push_back(mapListData);
    }
    else
    {
      maps.push_back(mapListData);
    }
  }

  pinnedMaps.insert(pinnedMaps.end(), maps.begin(), maps.end());

  for(auto const & map : pinnedMaps)
  {
    auto mapListItem = new Widget::MapListItem(map, parent->_continents_table);
    auto item = new QListWidgetItem(parent->_continents_table);

    if (map.Pinned)
    {
        QObject::connect(mapListItem, &QListWidget::customContextMenuRequested,
            [=](const QPoint& pos)
            {
                QMenu contextMenu(mapListItem->tr("Context menu"), mapListItem);

                QAction action1("Unpin Map", mapListItem);
                auto icon = QIcon();
                icon.addPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16, 16)));
                action1.setIcon(icon);

                QObject::connect(&action1, &QAction::triggered, [=]()
                {
                	parent->HandleEventMapListContextMenuUnpinMap(map.MapId);
                });

                contextMenu.addAction(&action1);
                contextMenu.exec(mapListItem->mapToGlobal(pos));
            });
    }
    else
    {
        QObject::connect(mapListItem, &QListWidget::customContextMenuRequested,
            [=](const QPoint& pos)
            {
                QMenu contextMenu(mapListItem->tr("Context menu"), mapListItem);
                QAction action1("Pin Map", mapListItem);
                auto icon = QIcon();
                icon.addPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16, 16)));
                action1.setIcon(icon);

                QObject::connect(&action1, &QAction::triggered, [=]()
                {
                	parent->HandleEventMapListContextMenuPinMap(map.MapId, map.MapName.toStdString());
                });

                contextMenu.addAction(&action1);
                contextMenu.exec(mapListItem->mapToGlobal(pos));
            });
    }

    item->setSizeHint(mapListItem->minimumSizeHint());
    item->setData(Qt::UserRole, QVariant(map.MapId));
    parent->_continents_table->setItemWidget(item, mapListItem);
  }

  parent->_project->ClientDatabase->UnloadTable(table);
}
