// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "DataListNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <vector>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

DataListNode::DataListNode()
: ListNodeBase()
{
  setName("List :: Create");
  setCaption("List :: Create");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);

  addPort<ListData>(PortType::Out, "List[Integer]", true);
  _out_ports[1].data_type->set_parameter_type("int");

  _type = new QComboBox(&_embedded_widget);
  _type->addItems(_type_list);

  QComboBox::connect(_type, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
       {
         auto new_type_id = _type_map[_type->currentText().toStdString()].c_str();

         if (_out_ports[1].data_type->type().parameter_type_id == new_type_id)
           return;

         setCaption("List[" + _type->currentText() + "]");
         deletePort(PortType::Out, 1);

         addPortDynamic<ListData>(PortType::Out, 1, _caption, true);

         _out_ports[1].data_type->set_parameter_type(new_type_id);
       }
  );

  addWidgetTop(_type);
}

void DataListNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  _data.clear();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  auto list =  std::make_shared<ListData>(&_data);
  list->set_parameter_type(_out_ports[1].data_type->type().parameter_type_id);
  _out_ports[1].out_value = std::move(list);

  _node->onDataUpdated(0);
  _node->onDataUpdated(1);
}

QJsonObject DataListNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["list_type"] = _type->currentText();

  return json_obj;
}

void DataListNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  auto list_type = json_obj["list_type"].toString();
  auto new_type_id = _type_map[list_type.toStdString()].c_str();
  _out_ports[1].data_type->set_parameter_type(new_type_id);
  _out_ports[1].caption = "List[" + list_type + "]";

  _type->setCurrentText(list_type);
}
