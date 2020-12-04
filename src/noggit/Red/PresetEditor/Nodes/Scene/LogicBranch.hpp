#ifndef NOGGIT_LOGICBRANCH_HPP
#define NOGGIT_LOGICBRANCH_HPP

#include <external/NodeEditor/include/nodes/FlowScene>
#include <external/NodeEditor/include/nodes/Node>
#include <vector>
#include <unordered_map>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::Node;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;
using QtNodes::Connection;


namespace noggit
{
    namespace Red::PresetEditor::Nodes
    {
        class LogicBranch
        {
        public:
            explicit LogicBranch(Node* logic_node);
            void processNode(Node* node);
            void executeNode(Node* node);
            void executeNodeLeaves(Node* node);
            void execute();

        private:
            Node* _logic_node;
            std::vector<Node*> _nodes;
            std::unordered_map<Node*, std::vector<LogicBranch>> _sub_branches;
        };

    }

}

#endif //NOGGIT_LOGICBRANCH_HPP
