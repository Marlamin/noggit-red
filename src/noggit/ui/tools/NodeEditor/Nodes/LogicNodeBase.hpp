// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOGICNODEBASE_HPP
#define NOGGIT_LOGICNODEBASE_HPP

#include "BaseNode.hpp"

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
            LogicNodeBase();
            void resetInterationIndex();
            unsigned getIterationindex() const;

            virtual bool isLogicNode() override;

            bool isIterable() const;

            void setNIterations(unsigned n_iterations);
            unsigned getNIteraitons() const;
            void setIterationIndex(unsigned index);

            NodeValidationState validate() override;

        protected:
            void setIsIterable(bool is_iterable);

            unsigned _iteration_index = 0;
            unsigned _n_iterations = 0;

            bool _is_iterable = false;
        };

    }

}

#endif //NOGGIT_LOGICNODEBASE_HPP
