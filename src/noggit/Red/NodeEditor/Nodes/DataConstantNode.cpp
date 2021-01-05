// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "DataConstantNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::NodeEditor::Nodes;

DataConstantNode::DataConstantNode()
: BaseNode()
{
  setName("DataConstantNode");
  setCaption("Integer");
  _validation_state = NodeValidationState::Valid;

  addPort<IntegerData>(PortType::In, "Integer", true);
  addDefaultWidget(_in_ports[0].data_type->default_widget(&_embedded_widget), PortType::In, 0);

  addPort<IntegerData>(PortType::Out, "Integer", true);

  _type = new QComboBox(&_embedded_widget);
  _type->addItems({"Integer",
                   "Unsigned Integer",
                   "Decimal",
                   "String",
                   "Boolean",
                   "Vector2D",
                   "Vector3D",
                   "Vector4D",
                   "Color"});

  QComboBox::connect(_type, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
     {
       setCaption(_type->currentText());
       deletePort(PortType::In, 0);
       deleteDefaultWidget(PortType::In, 0);
       deletePort(PortType::Out, 0);

       auto& type_id = _type_map.at(_type->currentText().toStdString());
       addPortDynamic<AnyData>(PortType::In, 0, _type->currentText(), true);
       addPortDynamic<AnyData>(PortType::Out, 0, _type->currentText(), true);

       _in_ports[0].data_type.reset(TypeFactory::create(type_id));
       _out_ports[0].data_type.reset(TypeFactory::create(type_id));

       addDefaultWidget(_in_ports[0].data_type->default_widget(&_embedded_widget), PortType::In, 0);

     }
  );

  addWidgetTop(_type);
}

void DataConstantNode::compute()
{

  auto value_shared = _in_ports[0].in_value.lock();

  if (value_shared)
    _out_ports[0].out_value = value_shared;
  else
    _out_ports[0].out_value = _in_ports[0].data_type->default_widget_data(_in_ports[0].default_widget);



  Q_EMIT dataUpdated(0);

}

QJsonObject DataConstantNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["type"] = _type->currentText();
  _in_ports[0].data_type->to_json(_in_ports[0].default_widget, json_obj, "value");

  return json_obj;
}

void DataConstantNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  auto type = json_obj["type"].toString();
  _type->setCurrentText(type);
  _in_ports[0].data_type->from_json(_in_ports[0].default_widget, json_obj, "value");

}

