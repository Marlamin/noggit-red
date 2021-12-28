// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/windows/noggitWindow/components/BuildMapListComponent.hpp>
#include <noggit/ui/windows/noggitWindow/widgets/MapListItem.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>

using namespace Noggit::Ui::Component;

void BuildMapListComponent::BuildMapList(Noggit::Ui::Windows::NoggitWindow* parent)
{
  parent->_continents_table->clear();

  const auto& table = std::string("Map");
  auto mapTable = parent->_project->ClientDatabase->LoadTable(table);

  auto iterator = mapTable.Records();
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

    auto mapListItem = new Widget::MapListItem(mapListData, parent->_continents_table);
    auto item = new QListWidgetItem(parent->_continents_table);
    item->setSizeHint(mapListItem->minimumSizeHint());
    item->setData(Qt::UserRole, QVariant(mapListData.MapId));
    parent->_continents_table->setItemWidget(item, mapListItem);
  }
  parent->_project->ClientDatabase->UnloadTable(table);
}

