#ifndef NOGGIT_NODESCENE_HPP
#define NOGGIT_NODESCENE_HPP

#include <external/NodeEditor/include/nodes/FlowScene>

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::Node;

namespace noggit
{
    namespace Red::NodeEditor::Nodes
    {
      class NodeScene : public FlowScene
      {
      public:
          NodeScene(std::shared_ptr<DataModelRegistry> registry,
                    QObject* parent = Q_NULLPTR) : FlowScene(std::move(registry), parent) {};
          bool execute();
          bool validate();

          Node* getBeginNode() {return _begin_node; };
          Node* getReturnNode() {return _return_node; };

      private:
          Node* _begin_node = nullptr;
          Node* _return_node = nullptr;
      };
    }
}

#endif //NOGGIT_NODESCENE_HPP
