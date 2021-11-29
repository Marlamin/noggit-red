#ifndef NOGGIT_LOGICBRANCH_HPP
#define NOGGIT_LOGICBRANCH_HPP

#include <external/NodeEditor/include/nodes/FlowScene>
#include <external/NodeEditor/include/nodes/Node>
#include <stack>

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
    namespace ui::tools::NodeEditor::Nodes
    {
        class LogicBranch
        {
        public:
            explicit LogicBranch(Node* logic_node);
            bool executeNode(Node* node, Node* source_node);
            static bool executeNodeLeaves(Node* node, Node* source_node);
            void markNodesComputed(Node* start_node, bool state);
            void markNodeLeavesComputed(Node* start_node, Node* source_node, bool state);
            void setCurrentLoop(Node* node) { _loop_stack.push(node); };
            void unsetCurrentLoop() { _loop_stack.pop(); };
            Node* getCurrentLoop() { return _loop_stack.empty() ? nullptr : _loop_stack.top(); };
            bool execute();

        private:
            Node* _logic_node;
            std::stack<Node*> _loop_stack;
            bool _return = false;
        };

    }

}

#endif //NOGGIT_LOGICBRANCH_HPP
