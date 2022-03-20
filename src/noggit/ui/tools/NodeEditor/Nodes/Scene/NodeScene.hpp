#ifndef NOGGIT_NODESCENE_HPP
#define NOGGIT_NODESCENE_HPP

#include <external/NodeEditor/include/nodes/FlowScene>
#include <external/tsl/robin_map.h>

#include "../DataTypes/GenericData.hpp"

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::Node;

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
      using VariableMap = tsl::robin_map<std::string, std::pair<std::string, std::shared_ptr<NodeData>>>;
      class NodeScene : public FlowScene
      {
      public:
          NodeScene(std::shared_ptr<DataModelRegistry> registry,
                    QObject* parent = Q_NULLPTR) : FlowScene(std::move(registry), parent) {};
          bool execute();
          bool validate();

          Node* getBeginNode() { return _begin_node; };
          Node* getReturnNode() { return _return_node; };
          VariableMap* getVariableMap() { return &_variables; };


      private:
          Node* _begin_node = nullptr;
          Node* _return_node = nullptr;
          VariableMap _variables;

      };
    }
}

#endif //NOGGIT_NODESCENE_HPP
