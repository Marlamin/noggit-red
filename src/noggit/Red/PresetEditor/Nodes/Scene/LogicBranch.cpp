#include "LogicBranch.hpp"
#include "../BaseNode.hpp"
#include "../LogicNodeBase.hpp"
#include "../LogicBreakNode.hpp"
#include "../LogicContinueNode.hpp"
#include "../Data/GenericData.hpp"

#include <stdexcept>

using namespace noggit::Red::NodeEditor::Nodes;

LogicBranch::LogicBranch(Node* logic_node)
: _logic_node(logic_node)
{
}


void LogicBranch::execute()
{
  _return = false;
  executeNode(_logic_node, nullptr);
}

void LogicBranch::executeNode(Node* node, Node* source_node)
{
  if (_return)
    return;

  auto model = static_cast<BaseNode*>(node->nodeDataModel());
  auto nodeState = node->nodeState();

  if (model->isComputed())
    return;

  model->compute();
  model->setComputed(true);

  // Handle loop breaking and continuing
  if(model->isLogicNode())
  {
    auto logic_node_model = static_cast<LogicNodeBase*>(model);

    if (logic_node_model->name() == "LogicBreakNode" && static_cast<LogicBreakNode*>(model)->doBreak())
    {
      auto break_node =  static_cast<LogicBreakNode*>(model);
      if (_loop_stack.empty())
      {
        break_node->setValidationState(NodeValidationState::Error);
        break_node->setValidationMessage("Error: break is outside any loop.");
      }
      else
      {
        Node* current_loop_node = getCurrentLoop();

        static_cast<LogicNodeBase*>(getCurrentLoop()->nodeDataModel())->setIterationIndex(-1);
        break_node->setDoBreak(false);
        markNodesComputed(current_loop_node, true);
      }
    }
    else if (logic_node_model->name() == "LogicContinueNode" && static_cast<LogicContinueNode*>(model)->doContinue())
    {
      auto continue_node =  static_cast<LogicContinueNode*>(model);
      if (_loop_stack.empty())
      {
        continue_node->setValidationState(NodeValidationState::Error);
        continue_node->setValidationMessage("Error: continue is outside any loop.");
      }
      else
      {
        Node* current_loop_node = getCurrentLoop();

        auto loop_model = static_cast<LogicNodeBase*>(current_loop_node->nodeDataModel());
        continue_node->setDoContinue(false);
        markNodesComputed(current_loop_node, true);
        loop_model->setComputed(false);
      }
    }
    else if (logic_node_model->name() == "LogicReturnNoDataNode")
    {
      _return = true;
      return;
    }
  }

  // do not continue further if validation or execution has found a problem
  if (model->validationState() == NodeValidationState::Error)
    return;

  // Handle dependant nodes
  for (int i = 0; i < model->nPorts(PortType::Out); ++i)
  {
    // we do not process dependant data nodes here, discard them
    if (model->dataType(PortType::Out, i).id != "logic")
      continue;

    // discard logic branches not suitable for evaluation
    if (!static_cast<LogicData*>(model->outData(i).get())->value())
      continue;

    auto const& connections = nodeState.connections(PortType::Out, i);

    for (auto const& pair : connections)
    {
      auto connected_node = pair.second->getNode(PortType::In);

      if (!connected_node)
        continue;

      auto connected_model = static_cast<BaseNode*>(connected_node->nodeDataModel());

      executeNodeLeaves(connected_node, node); // Execute data node leaves
      if (connected_model->validate() != NodeValidationState::Error)
      {
        auto logic_model = static_cast<LogicNodeBase*>(connected_node->nodeDataModel());

        if (logic_model->isIterable()) // handle iteration nodes
        {
          setCurrentLoop(connected_node);
          int it_index = logic_model->getIterationindex();
          while (it_index >= 0 && it_index < logic_model->getNIteraitons() && !_return)
          {
            markNodesComputed(connected_node, false);
            executeNode(connected_node, node);
            logic_model->setComputed(true);
            it_index = logic_model->getIterationindex();
          }
          unsetCurrentLoop();

        }
        else // haandle regular nodes
        {
          executeNode(connected_node, node);
        }

      }
    }
  }
}

void LogicBranch::executeNodeLeaves(Node* node, Node* source_node)
{
  auto model = static_cast<BaseNode*>(node->nodeDataModel());
  auto nodeState = node->nodeState();

  if (model->isComputed())
    return;

  for (int i = 0; i < model->nPorts(PortType::In); ++i)
  {
    auto const& connections = nodeState.connections(PortType::In, i);

    for (auto const& pair : connections)
    {
      auto connected_node = pair.second->getNode(PortType::Out);

      if (!connected_node)
        continue;

      auto connected_model = static_cast<BaseNode*>(connected_node->nodeDataModel());

      if (connected_node == source_node || connected_model->isComputed() || connected_node == getCurrentLoop())
        continue;

      if (connected_model->isLogicNode())
      {
        connected_model->setValidationState(NodeValidationState::Error);
        connected_model->setValidationMessage("Error: Logic node is out of logic flow.");
        continue;
      }

      executeNodeLeaves(connected_node, node);
      connected_model->compute();
    }
  }
}

void LogicBranch::markNodesComputed(Node* start_node, bool state)
{
  auto model = static_cast<BaseNode*>(start_node->nodeDataModel());
  auto nodeState = start_node->nodeState();

  model->setComputed(state);

  for (int i = 0; i < model->nPorts(PortType::Out); ++i)
  {
    auto const& connections = nodeState.connections(PortType::Out, i);

    for (auto const& pair : connections)
    {
      auto connected_node = pair.second->getNode(PortType::In);

      if (!connected_node)
        continue;

      markNodeLeavesComputed(connected_node, start_node, state);
      markNodesComputed(connected_node, state);


    }

  }
}

void LogicBranch::markNodeLeavesComputed(Node* start_node, Node* source_node, bool state)
{
  auto model = static_cast<BaseNode*>(start_node->nodeDataModel());
  auto nodeState = start_node->nodeState();

  model->setComputed(state);

  for (int i = 0; i < model->nPorts(PortType::In); ++i)
  {
    auto const& connections = nodeState.connections(PortType::In, i);


    for (auto const& pair : connections)
    {
      auto connected_node = pair.second->getNode(PortType::Out);
      auto connected_model = static_cast<BaseNode*>(connected_node->nodeDataModel());

      if (!connected_node || connected_node == source_node || connected_model->isLogicNode())
        continue;

      markNodeLeavesComputed(connected_node, start_node, state);
    }
  }
}
