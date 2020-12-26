// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "DataListNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

#include <vector>

using namespace noggit::Red::NodeEditor::Nodes;

DataListNode::DataListNode()
: LogicNodeBase()
{
  setName("DataListNode");
  setCaption("List[Integer]");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);

  addPort<ListData>(PortType::Out, "List[Integer]", true);
  _out_ports[1].data_type->set_parameter_type("int");

  _type = new QComboBox(&_embedded_widget);

  _type->addItems({"Integer",
                   "Unsigned Integer",
                   "Boolean",
                   "Decimal",
                   "String",
                   "Vector2D",
                   "Vector3D",
                   "Vector4D",
                   "Matrix4x4",
                   "Matrix3x3",
                   "Quaternion",
                   "Procedure"
                  });

  QComboBox::connect(_type, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
       {
         auto new_type_id = _type_map[_type->currentText().toStdString()].c_str();

         if (_out_ports[1].data_type->type().parameter_type_id == new_type_id)
           return;

         setCaption("List[" + _type->currentText() + "]");
         deletePort(PortType::Out, 1);

         auto& type_id = _type_map.at(_type->currentText().toStdString());
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
  _out_ports[1].out_value = std::make_shared<ListData>(&_data);

  Q_EMIT dataUpdated(0);
  Q_EMIT dataUpdated(1);
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


