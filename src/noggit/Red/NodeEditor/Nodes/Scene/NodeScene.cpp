#include "NodeScene.hpp"
#include "LogicBranch.hpp"
#include "noggit/Red/NodeEditor/Nodes/BaseNode.hpp"

#include <external/NodeEditor/include/nodes/Node>
#include <noggit/Log.h>

using namespace noggit::Red::NodeEditor::Nodes;


bool NodeScene::execute()
{
  if (!validate())
    return false;

  auto main_branch = LogicBranch(_begin_node);
  return main_branch.execute();

  _variables.clear();
}

bool NodeScene::validate()
{
  _begin_node = nullptr;
  _return_node = nullptr;

  for (auto& pair : _nodes)
  {
    auto model = static_cast<BaseNode*>(pair.second.get()->nodeDataModel());
    model->setComputed(false);

    if (model->isLogicNode())
    {
      if (model->getInterpreterToken() == NodeInterpreterTokens::BEGIN)
      {
        if (_begin_node)
        {
          LogError << "Error: Found more than one begin node in the script." << std::endl;
          return false;
        }

        _begin_node = pair.second.get();
      }

      if (model->getInterpreterToken() == NodeInterpreterTokens::RETURN)
      {
        if (_return_node)
        {
          LogError << "Error: Found more than one data return node in the script." << std::endl;
          return false;
        }

        _return_node = pair.second.get();
      }

    }

  }

  if (!_begin_node)
  {
    LogError << "Error: No entry point found in the script." << std::endl;
    return false;
  }

  return true;
}
