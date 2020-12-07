#include "LogicBranch.hpp"
#include "../BaseNode.hpp"
#include "../LogicNodeBase.hpp"
#include "../LogicBreakNode.hpp"

#include <stdexcept>

using namespace noggit::Red::PresetEditor::Nodes;

LogicBranch::LogicBranch(Node* logic_node)
: _logic_node(logic_node)
{
}


void LogicBranch::execute()
{
  executeNode(_logic_node, nullptr);
}

void LogicBranch::executeNode(Node* node, Node* source_node)
{
  auto model = static_cast<BaseNode*>(node->nodeDataModel());
  auto nodeState = node->nodeState();

  if (model->isComputed())
    return;

  model->compute();
  model->setComputed(true);

  if(model->isLogicNode()
    && static_cast<LogicNodeBase*>(model)->name() == "LogicBreakNode"
    && static_cast<LogicBreakNode*>(model)->doBreak())
  {
    static_cast<LogicNodeBase*>(getCurrentLoop()->nodeDataModel())->setIterationIndex(-1);
    static_cast<LogicBreakNode*>(model)->setDoBreak(false);
  }

  for (int i = 0; i < model->nPorts(PortType::Out); ++i)
  {
    auto const& connections = nodeState.connections(PortType::Out, i);

    for (auto const& pair : connections)
    {
      auto connected_node = pair.second->getNode(PortType::In);

      if (!connected_node)
        continue;

      auto connected_model = static_cast<BaseNode*>(connected_node->nodeDataModel());
      if (connected_model->isLogicNode())
      {
        executeNodeLeaves(connected_node, node);
        if (connected_model->validate() != NodeValidationState::Error)
        {
          auto logic_model = static_cast<LogicNodeBase*>(connected_node->nodeDataModel());

          if (logic_model->isIterable())
          {
            setCurrentLoop(connected_node);
            int it_index = logic_model->getIterationindex();
            while (it_index >= 0 && it_index < logic_model->getNIteraitons())
            {
              markNodesComputed(connected_node, false);
              executeNode(connected_node, node);
              logic_model->setComputed(true);
              it_index = logic_model->getIterationindex();
            }
            unsetCurrentLoop();

          }
          else
          {
            executeNode(connected_node, node);
          }
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
