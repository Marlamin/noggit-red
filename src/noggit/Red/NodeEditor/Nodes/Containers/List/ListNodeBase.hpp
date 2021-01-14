// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LISTNODEBASE_HPP
#define NOGGIT_LISTNODEBASE_HPP

#include "noggit/Red/NodeEditor/Nodes/LogicNodeBase.hpp"
#include <external/tsl/robin_map.h>

#include <QStringList>

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
    class ListNodeBase : public LogicNodeBase
    {
    Q_OBJECT

    public:
      ListNodeBase() : LogicNodeBase() {};

    protected:
      tsl::robin_map<std::string, std::string> _type_map = {
          {   {"Integer", "int"},
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
              {"Color", "color"},
              {"Noise", "noise"},
              {"Image", "image"},


          }
      };

      QStringList _type_list = {"Integer",
                               "Unsigned Integer",
                               "Boolean",
                               "Decimal",
                               "String",
                               "Vector2D",
                               "Vector3D",
                               "Vector4D",
                               "Matrix4x4",
                               "Matrix3x3",
                               "Quaternion",
                               "Procedure",
                               "Color",
                               "Noise",
                               "Image"
      };

    };

  }

}

#endif //NOGGIT_LISTNODEBASE_HPP
