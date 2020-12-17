#include "NodeScene.hpp"
#include "LogicBranch.hpp"
#include "../BaseNode.hpp"

#include <external/NodeEditor/include/nodes/Node>
#include <noggit/Log.h>

using namespace noggit::Red::NodeEditor::Nodes;


void NodeScene::execute()
{
  _begin_node = nullptr;
  _return_node = nullptr;

  for (auto& pair : _nodes)
  {
    auto model = static_cast<BaseNode*>(pair.second.get()->nodeDataModel());
    model->setComputed(false);

    if (model->isLogicNode())
    {
      if (model->name() == "LogicBeginNode")
      {
        if (_begin_node)
        {
          LogError << "Found more than one begin node in executed script. Aborting execution." << std::endl;
          return;
        }

        _begin_node = pair.second.get();
      }

      if (model->name() == "LogicReturnNode")
      {
        if (_return_node)
        {
          LogError << "Found more than one data return node in executed script. Aborting execution." << std::endl;
          return;
        }

        _return_node = pair.second.get();
      }

    }

  }

  if (!_begin_node)
  {
    LogError << "No entry point found in the executed script. Aborting execution." << std::endl;
    return;
  }

  auto main_branch = LogicBranch(_begin_node);
  main_branch.execute();
}
