#ifndef NOGGIT_NODESCENE_HPP
#define NOGGIT_NODESCENE_HPP

#include <external/NodeEditor/include/nodes/FlowScene>

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::Node;

namespace noggit
{
    namespace Red::PresetEditor::Nodes
    {
      class NodeScene : public FlowScene
      {
      public:
          NodeScene(std::shared_ptr<DataModelRegistry> registry,
                    QObject* parent = Q_NULLPTR) : FlowScene(std::move(registry), parent) {};
          void execute();

      private:
          Node* _begin_node = nullptr;
          Node* _return_node = nullptr;
      };
    }
}

#endif //NOGGIT_NODESCENE_HPP
