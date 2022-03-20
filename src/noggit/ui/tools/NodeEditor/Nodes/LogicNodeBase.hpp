// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOGICNODEBASE_HPP
#define NOGGIT_LOGICNODEBASE_HPP

#include "BaseNode.hpp"
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::Node;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class LogicNodeBase : public BaseNode
        {
        Q_OBJECT

        public:
            LogicNodeBase() {};
            void resetInterationIndex() { _iteration_index = 0; };
            unsigned getIterationindex() { return _iteration_index; };

            virtual bool isLogicNode() override { return true; };

            bool isIterable() { return _is_iterable; };

            void setNIterations(unsigned n_iterations) { _n_iterations = n_iterations; };
            unsigned getNIteraitons() { return _n_iterations; };
            void setIterationIndex(unsigned index) { _iteration_index = index; };

            NodeValidationState validate() override
            {
              setValidationState(NodeValidationState::Valid);
              auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

              if (!logic)
              {
                setValidationState(NodeValidationState::Error);
                setValidationMessage("Error: Failed to evaluate logic input");

                _out_ports[0].out_value = std::make_shared<LogicData>(false);
                _node->onDataUpdated(0);
              }

              return _validation_state;
            };


        protected:

            void setIsIterable(bool is_iterable) { _is_iterable = is_iterable; };

            unsigned _iteration_index = 0;
            unsigned _n_iterations = 0;

            bool _is_iterable = false;


        };

    }

}

#endif //NOGGIT_LOGICNODEBASE_HPP
