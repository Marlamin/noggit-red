#include "NodeScene.hpp"
#include "LogicBranch.hpp"
#include "../BaseNode.hpp"

#include <external/NodeEditor/include/nodes/Node>
#include <noggit/Log.h>

using namespace noggit::Red::PresetEditor::Nodes;

using QtNodes::Node;

void NodeScene::execute()
{
  Node* begin = nullptr;
  for (auto& pair : _nodes)
  {
    auto model = static_cast<BaseNode*>(pair.second.get()->nodeDataModel());
    model->setComputed(false);

    if (model->isLogicNode() && model->name() == "LogicBeginNode")
    {
      begin = pair.second.get();
    }

  }

  if (!begin)
  {
    LogError << "No entry point found in the executed script. Aborting execution." << std::endl;
    return;
  }

  auto main_branch = LogicBranch(begin);
  main_branch.execute();
}
