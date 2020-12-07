// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ForLoopNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::PresetEditor::Nodes;

ForLoopNode::ForLoopNode()
: LogicNodeBase()
{
  setName("ForLoopNode");
  setCaption("Repeat");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addWidget(new QLabel(&_embedded_widget), 0);
  addPort<IntegerData>(PortType::In, "Times", true);

  _n_iterations_default = new QSpinBox(&_embedded_widget);
  _n_iterations_default->setMinimum(0);
  addWidget(_n_iterations_default, 1);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
  addPort<IntegerData>(PortType::Out, "Index", true, ConnectionPolicy::Many);

  setIsIterable(true);
}

void ForLoopNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if(!logic->value())
  {
    setLogicBranchToExecute(-1);
    return;
  }

  auto n_iterations_ptr = static_cast<IntegerData*>(_in_ports[1].in_value.lock().get());

  int n_iterations = n_iterations_ptr ? n_iterations_ptr->value() : _n_iterations_default->value();

  if (!n_iterations)
  {
    setIterationIndex(-1);
    return;
  }

  if (_iteration_index == n_iterations)
  {
    setIterationIndex(-1);
    return;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<IntegerData>(getIterationindex());
  Q_EMIT dataUpdated(1);

  setIterationIndex(getIterationindex() + 1);

}

QJsonObject ForLoopNode::save() const
{
  QJsonObject json_obj;

  json_obj["name"] = name();
  json_obj["caption"] = caption();
  json_obj["n_iterations"] = _n_iterations_default->value();

  return json_obj;
}

void ForLoopNode::restore(const QJsonObject& json_obj)
{
  setName(json_obj["name"].toString());
  setCaption(json_obj["caption"].toString());
  _n_iterations_default->setValue(json_obj["n_iterations"].toInt());
}

NodeValidationState ForLoopNode::validate()
{
  setValidationState(NodeValidationState::Valid);

  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate logic input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    Q_EMIT dataUpdated(0);

    setLogicBranchToExecute(-1);
  }

  auto n_iterations_ptr = static_cast<IntegerData*>(_in_ports[1].in_value.lock().get());

  int n_iterations = n_iterations_ptr ? n_iterations_ptr->value() : _n_iterations_default->value();

  if (!n_iterations)
  {
    setIterationIndex(-1);
  }

  setIterationIndex(0);
  setNIterations(std::max(0, n_iterations));

  return _validation_state;
}

