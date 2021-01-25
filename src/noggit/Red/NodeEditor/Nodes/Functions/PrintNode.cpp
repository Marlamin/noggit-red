#include "PrintNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/Log.h>

using namespace noggit::Red::NodeEditor::Nodes;

PrintNode::PrintNode()
: LogicNodeBase()
{
  setName("Functions :: Print");
  setCaption("Print()");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);

  addPort<StringData>(PortType::In, "String", true);
  _text = new QLineEdit(&_embedded_widget);
  addDefaultWidget(_text, PortType::In, 1);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);

}

void PrintNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if(!logic->value())
    return;

  auto text = _in_ports[1].in_value.lock();
  auto text_ptr = static_cast<StringData*>(text.get());

  auto msg = text_ptr ? text_ptr->value() : _text->text().toStdString();

  LogDebug << msg << std::endl;

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);
}

QJsonObject PrintNode::save() const
{
  QJsonObject json_obj;
  json_obj["name"] = name();
  json_obj["caption"] = caption();
  json_obj["text"] = _text->text();

  return json_obj;
}

void PrintNode::restore(const QJsonObject& json_obj)
{
  setName(json_obj["name"].toString());
  setCaption(json_obj["caption"].toString());
  _text->setText(json_obj["text"].toString());
}

NodeValidationState PrintNode::validate()
{
  setValidationState(NodeValidationState::Valid);
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate logic input");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _node->onDataUpdated(0);
  }

  return _validation_state;

}
