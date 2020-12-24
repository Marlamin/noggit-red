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
  _type->addItems({"Integer", "Unsigned Integer", "Decimal", "String", "Boolean"});

  QComboBox::connect(_type, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
     {
       setCaption(_type->currentText());
       deletePort(PortType::In, 0);
       deleteDefaultWidget(PortType::In, 0);
       deletePort(PortType::Out, 0);

       switch (index)
       {
         case 0:
           addPortDynamic<IntegerData>(PortType::In, 0, "Integer", true);
           addPortDynamic<IntegerData>(PortType::Out, 0, "Integer", true);
           break;

         case 1:
           addPortDynamic<UnsignedIntegerData>(PortType::In, 0, "UInteger", true);
           addPortDynamic<UnsignedIntegerData>(PortType::Out, 0, "UInteger", true);
           break;

         case 2:
           addPortDynamic<DecimalData>(PortType::In, 0, "Decimal", true);
           addPortDynamic<DecimalData>(PortType::Out, 0, "Decimal", true);
           break;

         case 3:
           addPortDynamic<StringData>(PortType::In, 0, "String", true);
           addPortDynamic<StringData>(PortType::Out, 0, "String", true);
           break;

         case 4:
           addPortDynamic<BooleanData>(PortType::In, 0, "Boolean", true);
           addPortDynamic<BooleanData>(PortType::Out, 0, "Boolean", true);
           break;
       }

       addDefaultWidget(_in_ports[0].data_type->default_widget(&_embedded_widget), PortType::In, 0);

     }
  );

  addWidgetTop(_type);
}

void DataConstantNode::compute()
{

  auto value_shared = _in_ports[0].in_value.lock();

  switch (_type->currentIndex())
  {
    case 0:

      if (value_shared)
        _out_ports[0].out_value = std::make_shared<IntegerData>(static_cast<IntegerData*>(value_shared.get())->value());
      else
        _out_ports[0].out_value = _in_ports[0].data_type->default_widget_data(_in_ports[0].default_widget);

      break;

    case 1:
      if (value_shared)
        _out_ports[0].out_value = std::make_shared<UnsignedIntegerData>(static_cast<UnsignedIntegerData*>(value_shared.get())->value());
      else
        _out_ports[0].out_value = _in_ports[0].data_type->default_widget_data(_in_ports[0].default_widget);

      break;

    case 2:
      if (value_shared)
        _out_ports[0].out_value = std::make_shared<DecimalData>(static_cast<DecimalData*>(value_shared.get())->value());
      else
        _out_ports[0].out_value = _in_ports[0].data_type->default_widget_data(_in_ports[0].default_widget);

      break;

    case 3:
      if (value_shared)
        _out_ports[0].out_value = std::make_shared<StringData>(static_cast<StringData*>(value_shared.get())->value());
      else
        _out_ports[0].out_value = _in_ports[0].data_type->default_widget_data(_in_ports[0].default_widget);

      break;

    case 4:
      if (value_shared)
        _out_ports[0].out_value = std::make_shared<BooleanData>(static_cast<BooleanData*>(value_shared.get())->value());
      else
        _out_ports[0].out_value = _in_ports[0].data_type->default_widget_data(_in_ports[0].default_widget);

      break;
  }

  Q_EMIT dataUpdated(0);

}

QJsonObject DataConstantNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["type"] = _type->currentText();

  return json_obj;
}

void DataConstantNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  auto type = json_obj["type"].toString();
  _type->setCurrentText(type);
}

