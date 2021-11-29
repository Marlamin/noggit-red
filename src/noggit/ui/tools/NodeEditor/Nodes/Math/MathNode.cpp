#include "MathNode.hpp"
#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodeScene.hpp>
#include <external/NodeEditor/include/nodes/Node>
#include <cmath>
#include <algorithm>

using namespace noggit::ui::tools::NodeEditor::Nodes;
using QtNodes::Node;


MathNode::MathNode()
: BaseNode()
{
  _validation_state = NodeValidationState::Valid;
  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Add",
                        "Subtract",
                        "Multiply",
                        "Divide",
                        "Modulo",
                        "Min",
                        "Max",
                        "Pow"});

  addWidgetTop(_operation);

  setName("MathNode");
  setCaption("Math :: Add");

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
          ,[this](int index)
          {
            setCaption("Math :: " + _operation->currentText());
          }
  );

  addPort<BasicData>(PortType::In, "Basic", true);

  addPort<BasicData>(PortType::In, "Basic", true);

  addPort<UndefinedData>(PortType::Out, "Undefined", true);
}

void MathNode::compute()
{

  auto first_data_ptr = _in_ports[0].in_value.lock().get();
  auto second_data_ptr = _in_ports[1].in_value.lock().get();

  switch (_result_type)
  {
    case 0: // Int
    {
      int first = commonCast<int>(first_data_ptr, _first_type);
      int second = commonCast<int>(second_data_ptr, _second_type);
      handleOperation(first, second);
      break;
    }
    case 1: // Unsigned Int
    {
      unsigned first = commonCast<unsigned int>(first_data_ptr, _first_type);
      unsigned second = commonCast<unsigned int>(second_data_ptr, _second_type);
      handleOperation(first, second);
      break;
    }
    case 2: // Decimal
    {
      double first = commonCast<double>(first_data_ptr, _first_type);
      double second = commonCast<double>(second_data_ptr, _second_type);
      handleOperation(first, second);
      break;
    }
    case 3: // String
    {
      std::string first = commonCast<std::string>(first_data_ptr, _first_type);
      std::string second = commonCast<std::string>(second_data_ptr, _second_type);
      handleOperation(std::move(first), std::move(second));
      break;
    }
  }

}

QJsonObject MathNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();
  json_obj["first_type"] = _first_type;
  json_obj["second_type"] = _second_type;
  json_obj["result_type"] = _result_type;

  return json_obj;
}

void MathNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  _operation->setCurrentIndex(json_obj["operation"].toInt());
  _first_type = json_obj["first_type"].toInt();
  _second_type = json_obj["second_type"].toInt();
  _result_type = json_obj["result_type"].toInt();

  if (_first_type >= 0)
  {
    _in_ports[0].data_type.reset(TypeFactory::create(_types[_first_type].first.data()));
    _in_ports[0].caption = _in_ports[0].data_type->type().name;
  }

  if (_second_type >= 0)
  {
    _in_ports[1].data_type.reset(TypeFactory::create(_types[_second_type].first.data()));
    _in_ports[1].caption = _in_ports[1].data_type->type().name;
  }

  if (_result_type >= 0)
  {
    _out_ports[0].data_type.reset(TypeFactory::create(_types[_result_type].first.data()));
    _out_ports[0].caption = _out_ports[0].data_type->type().name;
  }

}

NodeValidationState MathNode::validate()
{
  setValidationState(NodeValidationState::Valid);

  if (!_in_ports[0].connected || !_in_ports[1].connected
      || _out_ports[0].data_type->type().id == "undefined")
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: result of operation is of undefined type.");
    return _validation_state;
  }

  return _validation_state;

}

void MathNode::calculateResultType(PortIndex port_index, PortIndex other_port_index)
{
  int first_temp = std::find_if(_types.cbegin(), _types.cend(), [this, port_index] (std::pair<std::string_view, int> const& pair) -> bool
                                {
                                    return pair.first == _in_ports[port_index].data_type->type().id.toStdString();
                                }
  )->second;

  int second_temp = std::find_if(_types.cbegin(), _types.cend(), [this, other_port_index] (std::pair<std::string_view, int> const& pair) -> bool
                                 {
                                     return pair.first == _in_ports[other_port_index].data_type->type().id.toStdString();
                                 }
  )->second;

  if (port_index)
  {
    _second_type = first_temp;
    _first_type = second_temp;
  }
  else
  {
    _second_type = second_temp;
    _first_type = first_temp;
  }

  _result_type = std::max(_first_type, _second_type);
}

void MathNode::inputConnectionCreated(const Connection& connection)
{
  BaseNode::inputConnectionCreated(connection);

  PortIndex port_index = connection.getPortIndex(PortType::In);
  PortIndex other_port_index = port_index ? 0 : 1;

  auto data_type = connection.dataType(PortType::Out);

  if (_in_ports[port_index].data_type->type().id != data_type.id)
  {
    _in_ports[port_index].data_type.reset(TypeFactory::create(data_type.id.toStdString()));
    _in_ports[port_index].caption = data_type.name;
  }

  if (_in_ports[other_port_index].connected)
  {

    calculateResultType(port_index, other_port_index);

    std::string_view result_type = std::find_if(_types.cbegin(), _types.cend(), [this] (std::pair<std::string_view, int> const& pair) -> bool
    {
        return pair.second == _result_type;

    })->first;

    if (_out_ports[0].connected && _out_ports[0].data_type->type().id != result_type.data())
    {
      for (auto &con : _node->nodeState().connections(PortType::Out, 0))
      {
        static_cast<NodeScene*>(_node->nodeGraphicsObject().scene())->deleteConnection(*con.second);
      }
    }
    _out_ports[0].data_type.reset(TypeFactory::create(result_type.data()));
    _out_ports[0].caption = _out_ports[0].data_type->type().name;
  }
}

void MathNode::inputConnectionDeleted(const Connection& connection)
{
  PortIndex port_index = connection.getPortIndex(PortType::In);

  if (port_index)
    _second_type = -1;
  else
    _first_type = -1;

  _in_ports[port_index].data_type.reset(TypeFactory::create("basic"));
  _in_ports[port_index].caption = "Basic";

  if (_out_ports[0].connected)
  {
    for (auto& con : _node->nodeState().connections(PortType::Out, 0))
    {
      static_cast<NodeScene*>(_node->nodeGraphicsObject().scene())->deleteConnection(*con.second);
    }
  }

  _out_ports[0].data_type.reset(TypeFactory::create("undefined"));
  _out_ports[0].caption = "Undefined";

  BaseNode::inputConnectionDeleted(connection);

}

template<typename T>
T MathNode::commonCast(NodeData* data, int type_id)
{

  if constexpr (std::is_same<T, std::string>::value)
  {
    switch (type_id)
    {
      case 0:
        return std::to_string(static_cast<IntegerData*>(data)->value());
        break;
      case 1:
        return std::to_string(static_cast<UnsignedIntegerData*>(data)->value());
        break;
      case 2:
        return std::to_string(static_cast<DecimalData*>(data)->value());
        break;
      case 3:
        return static_cast<StringData*>(data)->value();
        break;
    }
  }
  else
  {
    switch (type_id)
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
  typedef std::remove_reference_t<T> Type;

  Type result;

  if constexpr (!std::is_same<Type, std::string>::value)
  {
    switch (_operation->currentIndex())
    {
      case 0: // Add
        result = first + second;
        break;
      case 1: // Subtract
        result = first - second;
        break;
      case 2: // Multiply
        result = first * second;
        break;
      case 3: // Divide
        if constexpr (std::is_same<Type, int>::value || std::is_same<Type, unsigned int>::value)
        {
          if (!second)
          {
            setValidationState(NodeValidationState::Error);
            setValidationMessage("Error: division by zero");
            return;
          }

          result = first / second;
        }
        else
        {
          result = first / second;
        }
        break;
      case 4: // Modulo
        if constexpr (std::is_same<Type, int>::value || std::is_same<Type, unsigned int>::value)
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
          result = std::fmod(first, second);
        }
        break;

      case 5: // Min
        result = std::min(first, second);
        break;

      case 6: // Max
        result = std::max(first, second);
        break;

      case 7: // Pow
        result = std::pow(first, second);
        break;
    }
  }
  else
  {
    switch (_operation->currentIndex())
    {
      case 0: // Concatenate
        result = first + second;
        break;
      default:
        setValidationState(NodeValidationState::Error);
        setValidationMessage("Error: only Add is supported for strings.");
        return;
        break;
    }
  }

  if constexpr (std::is_same<Type, int>::value)
  {
    _out_ports[0].out_value = std::make_shared<IntegerData>(result);
  }
  else if constexpr (std::is_same<Type, unsigned int>::value)
  {
    _out_ports[0].out_value = std::make_shared<UnsignedIntegerData>(result);

  }
  else if constexpr (std::is_same<Type, double>::value)
  {
    _out_ports[0].out_value = std::make_shared<DecimalData>(result);
  }
  else if constexpr (std::is_same<Type, std::string>::value)
  {
    _out_ports[0].out_value = std::make_shared<StringData>(std::move(result));
  }

  _node->onDataUpdated(0);

}


