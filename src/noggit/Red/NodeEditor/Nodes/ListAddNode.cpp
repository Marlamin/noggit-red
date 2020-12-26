// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ListAddNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"
#include "Scene/NodeScene.hpp"

#include <external/NodeEditor/include/nodes/Node>

using QtNodes::Node;

using namespace noggit::Red::NodeEditor::Nodes;

ListAddNode::ListAddNode()
: LogicNodeBase()
{
  setName("ListAddNode");
  setCaption("Append");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Append", "Prepend", "Insert", "Set"});

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
     {
       setCaption(_operation->currentText());

       if (_in_ports.size() < 4 && index != 2)
         return;

       switch (index)
       {
         case 2:
         case 3:
           addPortDynamic<UnsignedIntegerData>(PortType::In, 3, "Index<UInteger>", true);
           addDefaultWidget(_in_ports[3].data_type->default_widget(&_embedded_widget), PortType::In, 3);
           break;

         default:
           deletePort(PortType::In, 3);
           break;
       }
     }
  );

  addWidgetTop(_operation);

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);

  addPort<ListData>(PortType::In, "List[Any]", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 1);

  addPort<UndefinedData>(PortType::In, "Value<Undefined>", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 2);

  addPort<LogicData>(PortType::Out, "Logic", true);

}

void ListAddNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get())->value();
  auto value = _in_ports[2].in_value.lock();

  switch (_operation->currentIndex())
  {
    case 0:
      list->push_back(value);
      break;
    case 1:
      list->insert(list->begin(), value);
      break;
    case 2:
    {
      auto index_ptr = static_cast<UnsignedIntegerData*>(_in_ports[3].in_value.lock().get());
      auto index = index_ptr ? index_ptr->value() : static_cast<QSpinBox*>(_in_ports[3].default_widget)->value();

      if (index < 0 || index > list->size())
      {
        setValidationState(NodeValidationState::Error);
        setValidationMessage("Error: invalid index.");
        return;
      }

      list->insert(list->begin() + index, value);
      break;
    }
    case 3:
    {
      auto index_ptr = static_cast<UnsignedIntegerData *>(_in_ports[3].in_value.lock().get());
      auto index = index_ptr ? index_ptr->value() : static_cast<QSpinBox*>(_in_ports[3].default_widget)->value();

      if (index < 0 || index >= list->size())
      {
        setValidationState(NodeValidationState::Error);
        setValidationMessage("Error: invalid index.");
        return;
      }

      (*list)[index] = value;
      break;
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

}

NodeValidationState ListAddNode::validate()
{
  LogicNodeBase::validate();

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get());

  if (!list)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate list input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    Q_EMIT dataUpdated(0);
  }

  auto value = static_cast<UndefinedData*>(_in_ports[2].in_value.lock().get());

  if (!value)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate value input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    Q_EMIT dataUpdated(0);
  }

  return _validation_state;
}

QJsonObject ListAddNode::save() const
{
  QJsonObject json_obj = LogicNodeBase::save();

  json_obj["operation"] = _operation->currentIndex();
  json_obj["list_type"] = _in_ports[1].data_type->type().parameter_type_id;

  return json_obj;
}

void ListAddNode::restore(const QJsonObject& json_obj)
{
  LogicNodeBase::restore(json_obj);

  auto type_id = json_obj["list_type"].toString();
  auto type = TypeFactory::create(type_id.toStdString());

  _in_ports[1].data_type->set_parameter_type(type_id);
  _in_ports[1].caption = "List<" + type->type().name + ">";

  _in_ports[2].data_type.reset(type);
  _in_ports[2].caption = "Value<" + _in_ports[2].data_type->type().name + ">";

  _operation->setCurrentIndex(json_obj["operation"].toInt());
}

void ListAddNode::inputConnectionCreated(const Connection& connection)
{
  BaseNode::inputConnectionCreated(connection);

  auto port_index = connection.getPortIndex(PortType::In);
  if (port_index == 1)
  {
    auto parameter_type = connection.dataType(PortType::Out).parameter_type_id;

    _in_ports[1].data_type->set_parameter_type(parameter_type);
    _in_ports[1].caption = connection.getNode(PortType::Out)->nodeDataModel()->portCaption(PortType::Out, connection.getPortIndex(PortType::Out));

    _in_ports[2].data_type.reset(TypeFactory::create(parameter_type.toStdString()));
    _in_ports[2].caption = "Value<" + _in_ports[2].data_type->type().name + ">";
  }
  else if (port_index == 3 && _operation->currentIndex() == 3)
  {
    addPortDynamic<UnsignedIntegerData>(PortType::In, 3, "Index", true);
    addDefaultWidget(_in_ports[3].data_type->default_widget(&_embedded_widget), PortType::In, 3);
  }

}

void ListAddNode::inputConnectionDeleted(const Connection& connection)
{
  BaseNode::inputConnectionDeleted(connection);

  auto port_index = connection.getPortIndex(PortType::In);
  if  (port_index == 1)
  {
    _in_ports[1].data_type->set_parameter_type("");
    _in_ports[1].caption = "List[Any]";

    _in_ports[2].data_type.reset(TypeFactory::create("undefined"));
    _in_ports[2].caption = "Value<Undefined>";

    // remove connection if List changes
    if (_in_ports[2].connected)
    {
      auto this_node = connection.getNode(PortType::In);
      auto connections = this_node->nodeState().connections(PortType::In, 2);

      for (auto& pair : connections)
      {
        static_cast<NodeScene*>(this_node->nodeGraphicsObject().scene())->deleteConnection(*pair.second);
      }
    }

  }

}

