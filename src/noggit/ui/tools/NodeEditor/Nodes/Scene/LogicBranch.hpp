#ifndef NOGGIT_LOGICBRANCH_HPP
#define NOGGIT_LOGICBRANCH_HPP

#include <stack>

namespace QtNodes
{
  class Node;
}

using QtNodes::Node;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class LogicBranch
        {
        public:
            explicit LogicBranch(Node* logic_node);
            bool executeNode(Node* node, Node* source_node);
            static bool executeNodeLeaves(Node* node, Node* source_node);
            void markNodesComputed(Node* start_node, bool state);
            void markNodeLeavesComputed(Node* start_node, Node* source_node, bool state);
            void setCurrentLoop(Node* node);
            void unsetCurrentLoop();
            Node* getCurrentLoop();
            bool execute();

        private:
            Node* _logic_node;
            std::stack<Node*> _loop_stack;
            bool _return = false;
        };
    }
}

#endif //NOGGIT_LOGICBRANCH_HPP
