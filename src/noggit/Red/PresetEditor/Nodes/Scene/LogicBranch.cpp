#include "LogicBranch.hpp"
#include "../BaseNode.hpp"

#include <stdexcept>

using namespace noggit::Red::PresetEditor::Nodes;

LogicBranch::LogicBranch(Node* logic_node)
: _logic_node(logic_node)
{
  processNode(logic_node);
}

void LogicBranch::processNode(Node* node)
{
  auto model = static_cast<BaseNode*>(node->nodeDataModel());
  auto nodeState = node->nodeState();

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
        for (unsigned int j = 0; j < connected_model->nLogicBranches(); ++j)
        {
          _sub_branches[connected_node].emplace_back(connected_node);
        }
      }
      else
      {
        _nodes.push_back(connected_node);
        processNode(connected_node);
      }
    }
  }
}

void LogicBranch::execute()
{
  executeNode(_logic_node);
}

void LogicBranch::executeNode(Node *node)
{
  auto model = static_cast<BaseNode*>(node->nodeDataModel());
  auto nodeState = node->nodeState();

  if (model->isComputed())
    return;

  executeNodeLeaves(node);
  model->compute();
  model->setComputed(true);

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
        executeNodeLeaves(connected_node);
        executeNode(connected_node);
      }
      else
      {
        executeNodeLeaves(connected_node);
        executeNode(connected_node);
      }
    }
  }
}

void LogicBranch::executeNodeLeaves(Node *node)
{
  auto model = static_cast<BaseNode *>(node->nodeDataModel());
  auto nodeState = node->nodeState();

  for (int i = 0; i < model->nPorts(PortType::In); ++i)
  {
    auto const& connections = nodeState.connections(PortType::In, i);

    for (auto const& pair : connections)
    {
      auto connected_node = pair.second->getNode(PortType::Out);

      if (!connected_node)
        continue;

      auto connected_model = static_cast<BaseNode*>(connected_node->nodeDataModel());

      if (connected_model->isComputed() || connected_node == node)
        continue;

      if (connected_model->isLogicNode())
      {
        connected_model->setValidationMessage("Error: A leaf should not represent a logic node!");
        connected_model->setValidationState(NodeValidationState::Error);
        connected_model->setComputed(true);
        continue;
      }
      else if (!connected_model->isComputed())
      {
        executeNodeLeaves(connected_node);
        connected_model->compute();
        connected_model->setComputed(true);
      }
    }
  }
}
