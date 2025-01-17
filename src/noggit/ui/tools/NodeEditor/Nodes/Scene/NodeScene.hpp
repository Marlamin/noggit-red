#ifndef NOGGIT_NODESCENE_HPP
#define NOGGIT_NODESCENE_HPP

#include <external/NodeEditor/include/nodes/FlowScene>
#include <external/tsl/robin_map.h>

namespace QtNodes
{
  class DataModelRegistry;
  class Node;
  class NodeData;
}

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::Node;

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
      using VariableMap = tsl::robin_map<std::string, std::pair<std::string, std::shared_ptr<QtNodes::NodeData>>>;
      class NodeScene : public FlowScene
      {
      public:
          NodeScene(std::shared_ptr<DataModelRegistry> registry,
                    QObject* parent = Q_NULLPTR);
          bool execute();
          bool validate();

          Node* getBeginNode();
          Node* getReturnNode();
          VariableMap* getVariableMap();


      private:
          Node* _begin_node = nullptr;
          Node* _return_node = nullptr;
          VariableMap _variables;

      };
    }
}

#endif //NOGGIT_NODESCENE_HPP
