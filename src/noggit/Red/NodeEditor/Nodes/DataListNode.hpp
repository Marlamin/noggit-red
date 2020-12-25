// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_DATALISTNODE_HPP
#define NOGGIT_DATALISTNODE_HPP

#include "LogicNodeBase.hpp"

#include <QComboBox>
#include <vector>
#include <external/tsl/robin_map.h>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace Red::NodeEditor::Nodes
    {
        class DataListNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            DataListNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QComboBox* _type;
            tsl::robin_map<std::string, std::string> _type_map = {
                {{"Integer", "int"},
                {"Unsigned Integer", "uint"},
                {"Boolean", "bool"},
                {"Decimal", "double"},
                {"String", "string"},
                {"Vector2D", "vec2"},
                {"Vector3D", "vec3"},
                {"Vector4D", "vec4"},
                {"Matrix4x4", "mat4"},
                {"Matrix3x3", "mat3"},
                {"Quaternion", "quat"},
                {"Procedure", "procedure"},
                }
            };

            std::vector<std::shared_ptr<NodeData>> _data;
        };

    }

}

#endif //NOGGIT_DATALISTNODE_HPP
