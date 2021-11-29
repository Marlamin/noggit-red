#include "NodeScene.hpp"
#include "LogicBranch.hpp"
#include "noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp"
#include "noggit/ui/tools/NodeEditor/Nodes/Scene/Context.hpp"

#include <external/NodeEditor/include/nodes/Node>
#include <noggit/Log.h>
#include <noggit/MapView.h>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

using namespace noggit::Red::NodeEditor::Nodes;


bool NodeScene::execute()
{
  if (!validate())
    return false;



  NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(gCurrentContext->getViewport()));
  auto main_branch = LogicBranch(_begin_node);
  bool result = main_branch.execute();
  _variables.clear();
  NOGGIT_ACTION_MGR->endAction();
  return result;
}

bool NodeScene::validate()
{
  _begin_node = nullptr;
  _return_node = nullptr;

  for (auto& pair : _nodes)
  {
    auto model = static_cast<BaseNode*>(pair.second.get()->nodeDataModel());
    model->setComputed(false);
    model->setValidationState(NodeValidationState::Valid);

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
