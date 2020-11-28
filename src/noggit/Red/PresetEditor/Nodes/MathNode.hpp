#ifndef NOGGIT_MATHNODE_HPP
#define NOGGIT_MATHNODE_HPP

#include "BaseNode.hpp"

#include <QDoubleSpinBox>
#include <QComboBox>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace Red::PresetEditor::Nodes
    {
        class MathNode : public BaseNode
        {
        Q_OBJECT

        public:
            MathNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QDoubleSpinBox* _first;
            QDoubleSpinBox* _second;
            QComboBox* _operation;

        };

    }

}


#endif //NOGGIT_MATHNODE_HPP
