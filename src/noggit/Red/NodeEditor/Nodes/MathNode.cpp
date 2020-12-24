#include "MathNode.hpp"
#include "BaseNode.inl"
#include "noggit/Red/NodeEditor/Nodes/Data/GenericData.hpp"
#include <cmath>
#include <stdexcept>

using namespace noggit::Red::NodeEditor::Nodes;


MathNode::MathNode()
: BaseNode()
{
  _validation_state = NodeValidationState::Valid;
  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Add",
                        "Subtract",
                        "Multiply",
                        "Divide",
                        "Modulo"});
  addWidgetTop(_operation);

  setName("MathNode");
  setCaption("Add");

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
          ,[this](int index)
          {
              setCaption(_operation->currentText());
          }
  );

  addPort<AnyData>(PortType::In, "Any*", true);

  addPort<AnyData>(PortType::In, "Any*", true);

  addPort<UndefinedData>(PortType::Out, "Undefined", true);
}

void MathNode::compute()
{
  auto first_shared = _in_ports[0].in_value.lock();
  auto second_shared = _in_ports[1].in_value.lock();

  auto first_type_id = _in_ports[0].data_type->type().id;
  auto second_type_id = _in_ports[1].data_type->type().id;

  int winning_type = std::max(_type_map[first_type_id.toStdString()], _type_map[second_type_id.toStdString()]);
  int winning_port = first_type_id < second_type_id;

  switch (winning_type)
  {
    case 0:
    {
      auto first = commonCast<int>(first_shared.get());
      auto second = commonCast<int>(second_shared.get());
      handleOperation(first, second);
      break;
    }
    case 1:
    {
      auto first = commonCast<unsigned int>(first_shared.get());
      auto second = commonCast<unsigned int>(second_shared.get());
      handleOperation(first, second);
      break;
    }
    case 2:
    {
      auto first = commonCast<double>(first_shared.get());
      auto second = commonCast<double>(second_shared.get());
      handleOperation(first, second);
      break;
    }
    case 3:
    {
      auto first = commonCast<std::string>(first_shared.get());
      auto second = commonCast<std::string>(second_shared.get());
      handleOperation(first, second);
      break;
    }
    default:
      break;
  }

}

QJsonObject MathNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  auto first_type_id = _in_ports[0].data_type->type().id;
  json_obj["first_type_id"] = first_type_id;

  auto second_type_id = _in_ports[1].data_type->type().id;
  json_obj["second_type_id"] = second_type_id;

  json_obj["ret_type_id"] = _out_ports[0].data_type->type().id;

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void MathNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  auto first_type_id = json_obj["first_type_id"].toString();

  _in_ports[0].data_type.reset(TypeFactory::create(first_type_id.toStdString()));
  _in_ports[0].caption = _in_ports[0].data_type->type().name;

  auto second_type_id = json_obj["second_type_id"].toString();

  _in_ports[1].data_type.reset(TypeFactory::create(second_type_id.toStdString()));
  _in_ports[1].caption = _in_ports[1].data_type->type().name;

  auto ret_type_id = json_obj["ret_type_id"].toString().toStdString();
  _out_ports[0].data_type.reset(TypeFactory::create(ret_type_id));
  _out_ports[0].caption = _out_ports[0].data_type->type().name;

  _operation->setCurrentIndex(json_obj["operation"].toInt());
}

void MathNode::inputConnectionCreated(const Connection& connection)
{
  BaseNode::inputConnectionCreated(connection);

  PortIndex port_index = connection.getPortIndex(PortType::In);
  PortIndex other_port_index = port_index ? 0 : 1;

  bool supported_type = false;

  // Disconnect types which are not supported by the node.
  if (_type_map.find(connection.dataType(PortType::Out).id.toStdString()) == _type_map.end())
  {
    deletePort(PortType::In, port_index);
    addPort<AnyData>(PortType::In, port_index, "Any*", true);
    emit portAdded(PortType::In, port_index);

    deletePort(PortType::Out, 0);
    addPort<UndefinedData>(PortType::Out, "Undefined", true);
    emit portAdded(PortType::Out, 0);
    return;
  }

  _in_ports[port_index].data_type.reset(TypeFactory::create(connection.dataType(PortType::Out).id.toStdString()));
  _in_ports[port_index].caption = _in_ports[port_index].data_type->type().name;

  auto other_type_id = _in_ports[other_port_index].data_type->type().id.toStdString();
  if (other_type_id != "any")
  {
    auto result_type_id = std::max(_type_map[_in_ports[port_index].data_type->type().id.toStdString()], _type_map[other_type_id]);

    std::string result_type_id_str;

    for (auto& it : _type_map)
    {
      if (it.second == result_type_id)
      {
        result_type_id_str = it.first;
        break;
      }
    }

    if (result_type_id_str == "string" && _operation->currentIndex())
    {
      setValidationState(NodeValidationState::Error);
      setValidationMessage("Error: String type only supports concatenation (Add).");
      deletePort(PortType::Out, 0);
      addPort<UndefinedData>(PortType::Out, 0, "Undefined", true);
      emit portAdded(PortType::Out, 0);
      return;
    }

    if (_out_ports[0].data_type->type().id.toStdString() != result_type_id_str)
    {
      deletePort(PortType::Out, 0);
      auto new_type = TypeFactory::create(result_type_id_str);
      addPort<UndefinedData>(PortType::Out, 0, new_type->type().name, true);
      _out_ports[0].data_type.reset(new_type);
      emit portAdded(PortType::Out, 0);
    }
  }

}

void MathNode::inputConnectionDeleted(const Connection& connection)
{
  BaseNode::inputConnectionDeleted(connection);

  PortIndex port_index = connection.getPortIndex(PortType::In);
  deletePort(PortType::In, port_index);
  addPort<AnyData>(PortType::In, port_index, "Any*", true);
  emit portAdded(PortType::In, port_index);

  deletePort(PortType::Out, 0);
  addPort<UndefinedData>(PortType::Out, "Undefined", true);
  emit portAdded(PortType::Out, 0);
}

NodeValidationState MathNode::validate()
{
  setValidationState(NodeValidationState::Valid);

  if (_out_ports[0].data_type->type().id == "undefined")
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: result of operation is of undefined type.");
    return _validation_state;
  }

  return _validation_state;
}

template<typename T>
T MathNode::commonCast(NodeData* data)
{
  int type_id_to;

  if constexpr (std::is_same<T, int>::value)
    type_id_to = 0;
  else if constexpr (std::is_same<T, unsigned int>::value)
    type_id_to = 1;
  else if constexpr (std::is_same<T, double>::value)
    type_id_to = 2;
  else if constexpr (std::is_same<T, std::string>::value)
    type_id_to = 3;

  if constexpr (std::is_same<T, std::string>::value)
  {
    switch (_type_map[data->type().id.toStdString()])
    {
      case 0:
        return std::move(std::to_string(static_cast<IntegerData*>(data)->value()));
        break;
      case 1:
        return std::move(std::to_string(static_cast<UnsignedIntegerData*>(data)->value()));
        break;
      case 2:
        return std::move(std::to_string(static_cast<DecimalData*>(data)->value()));
        break;
      case 3:
        return static_cast<StringData*>(data)->value();
        break;
    }
  }
  else
  {
    switch (_type_map[data->type().id.toStdString()])
    {
      case 0:
        return static_cast<T>(static_cast<IntegerData*>(data)->value());
        break;
      case 1:
        return static_cast<T>(static_cast<UnsignedIntegerData*>(data)->value());
        break;
      case 2:
        return static_cast<T>(static_cast<DecimalData*>(data)->value());
        break;
      case 3:
        throw std::logic_error("Casting String type to numeric types is not supported in MathNode.");
        break;
    }
  }

  throw std::logic_error("Invalid types for MathNode operation.");
}

template<typename T>
void MathNode::handleOperation(T first, T second)
{
  setValidationState(NodeValidationState::Warning);

  T result;

  if constexpr (!std::is_same<T, std::string>::value)
  {
    switch (_operation->currentIndex())
    {
      case 0:
        result = first + first;
        break;
      case 1:
        result = first - first;
        break;
      case 2:
        result = first * first;
        break;
      case 3:
        if constexpr (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
        {
          if (!second)
          {
            setValidationState(NodeValidationState::Error);
            setValidationMessage("Error: division by zero");
            return;
          }

          result = first / first;
        }
        else
        {
          result = first / first;
        }
        break;
      case 4:
        if constexpr (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
        {
          if (!second)
          {
            setValidationState(NodeValidationState::Error);
            setValidationMessage("Error: modulo by zero");
            return;
          }

          result = first % second;
        }
        else
        {
          result = std::fmod(first, first);
        }
        break;
    }
  }
  else
  {
    switch (_operation->currentIndex())
    {
      case 0:
        result = first + first;
        break;
      default:
        throw std::logic_error("String type only supports concatenation in MathNode.");
        break;
    }
  }

  if constexpr (std::is_same<T, int>::value)
  {
    _out_ports[0].out_value = std::make_shared<IntegerData>(result);

    setValidationMessage(("Debug: " + std::to_string(result)).c_str());

    Q_EMIT dataUpdated(0);
  }
  else if constexpr (std::is_same<T, unsigned int>::value)
  {
    _out_ports[0].out_value = std::make_shared<UnsignedIntegerData>(result);

    setValidationMessage(("Debug: " + std::to_string(result)).c_str());

    Q_EMIT dataUpdated(0);
  }
  else if constexpr (std::is_same<T, double>::value)
  {
    _out_ports[0].out_value = std::make_shared<DecimalData>(result);

    setValidationMessage(("Debug: " + std::to_string(result)).c_str());

    Q_EMIT dataUpdated(0);
  }
  else if constexpr (std::is_same<T, std::string>::value)
  {
    _out_ports[0].out_value = std::make_shared<StringData>(result);

    setValidationMessage(("Debug: " + result).c_str());

    Q_EMIT dataUpdated(0);
  }

}


