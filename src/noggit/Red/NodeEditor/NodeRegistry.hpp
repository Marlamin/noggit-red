// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NODEREGISTRY_HPP
#define NOGGIT_NODEREGISTRY_HPP

#include <external/NodeEditor/include/nodes/NodeData>
#include <external/NodeEditor/include/nodes/FlowScene>
#include <external/NodeEditor/include/nodes/FlowView>
#include <external/NodeEditor/include/nodes/ConnectionStyle>
#include <external/NodeEditor/include/nodes/TypeConverter>

#include <noggit/Red/NodeEditor/Nodes/MathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/MathUnaryNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/ColorMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ColorToRGBANode.hpp>
#include <noggit/Red/NodeEditor/Nodes/RGBAtoColorNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/VectorMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/VectorScalarMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Vector2DToXYNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Vector3DToXYZNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Vector4DToXYZWNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/XYtoVector2D.hpp>
#include <noggit/Red/NodeEditor/Nodes/XYZtoVector3DNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/XYZWtoVector4DNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/MatrixMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/MatrixDecomposeNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/MatrixUnaryMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/MatrixTransformNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/MatrixRotateQuaternionNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/ConditionNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicIfNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicBeginNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicBreakNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicContinueNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicReturnNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicReturnNoDataNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/PrintNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicForLoopNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicWhileLoopNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicChainNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/LogicProcedureNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/DataConstantNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/DataListNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ListAddNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ListGetNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ListSizeNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ListClearNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ListEraseNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ListReserveNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ListDeclareNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/GetVariableNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/GetVariableLazyNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/SetVariableNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/DeleteVariableNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/MatrixNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/LoadImageNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/ImageInfoNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/BaseNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/GenericTypeConverter.hpp>
#include <noggit/Red/NodeEditor/Nodes/Scene/NodeScene.hpp>

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;
using QtNodes::ConnectionStyle;
using QtNodes::TypeConverter;
using QtNodes::TypeConverterId;

#define REGISTER_TYPE_CONVERTER(T_FROM, T_TO)                       \
        registerTypeConverter(std::make_pair(T_FROM##Data().type(), \
        T_TO##Data().type()),                                       \
        TypeConverter{T_FROM##To##T_TO##TypeConverter()})

namespace noggit
{
    namespace Red::NodeEditor::Nodes
    {

        static std::shared_ptr<DataModelRegistry> registerDataModels()
        {
          auto ret = std::make_shared<DataModelRegistry>();

          // Math
          ret->registerModel<MathNode>("Math");
          ret->registerModel<MathUnaryNode>("Math");

          // Color Math
          ret->registerModel<ColorMathNode>("Math//Color");
          ret->registerModel<ColorToRGBANode>("Math//Color");
          ret->registerModel<RGBAtoColorNode>("Math//Color");

          // Vector Math
          ret->registerModel<Vector2DMathNode>("Math//Vector");
          ret->registerModel<Vector3DMathNode>("Math//Vector");
          ret->registerModel<Vector4DMathNode>("Math//Vector");
          ret->registerModel<Vector2DScalarMathNode>("Math//Vector");
          ret->registerModel<Vector3DScalarMathNode>("Math//Vector");
          ret->registerModel<Vector4DScalarMathNode>("Math//Vector");
          ret->registerModel<XYtoVector2DNode>("Math//Vector");
          ret->registerModel<XYZtoVector3DNode>("Math//Vector");
          ret->registerModel<XYZWtoVector4DNode>("Math//Vector");
          ret->registerModel<Vector2DToXYNode>("Math//Vector");
          ret->registerModel<Vector3DToXYZNode>("Math//Vector");
          ret->registerModel<Vector4DToXYZWNode>("Math//Vector");

          // Matrix
          ret->registerModel<MatrixMathNode>("Math//Matrix");
          ret->registerModel<MatrixDecomposeNode>("Math//Matrix");
          ret->registerModel<MatrixUnaryMathNode>("Math//Matrix");
          ret->registerModel<MatrixTransformNode>("Math//Matrix");
          ret->registerModel<MatrixRotateQuaternionNode>("Math//Matrix");

          // Data
          ret->registerModel<DataConstantNode>("Data");
          ret->registerModel<GetVariableNode>("Data");
          ret->registerModel<GetVariableLazyNode>("Data");
          ret->registerModel<GetContextVariableNode>("Data");
          ret->registerModel<GetContextVariableLazyNode>("Data");
          ret->registerModel<SetVariableNode>("Data");
          ret->registerModel<SetContextVariableNode>("Data");
          ret->registerModel<DeleteVariableNode>("Data");
          ret->registerModel<DeleteContextVariableNode>("Data");
          ret->registerModel<MatrixNode>("Data");

          // Image
          ret->registerModel<LoadImageNode>("Data//Image");
          ret->registerModel<ImageInfoNode>("Data//Image");

          // Logic
          ret->registerModel<ConditionNode>("Logic");
          ret->registerModel<LogicBeginNode>("Logic//Flow");
          ret->registerModel<LogicIfNode>("Logic//Flow");
          ret->registerModel<LogicForLoopNode>("Logic//Flow");
          ret->registerModel<LogicWhileLoopNode>("Logic//Flow");
          ret->registerModel<LogicBreakNode>("Logic//Flow");
          ret->registerModel<LogicContinueNode>("Logic//Flow");
          ret->registerModel<LogicChainNode>("Logic//Flow");
          ret->registerModel<LogicReturnNode>("Logic//Flow");
          ret->registerModel<LogicReturnNoDataNode>("Logic//Flow");
          ret->registerModel<LogicProcedureNode>("Logic//Flow");
          ret->registerModel<PrintNode>("Functions//Generic");

          // List
          ret->registerModel<DataListNode>("Containers//List");
          ret->registerModel<ListAddNode>("Containers//List");
          ret->registerModel<ListGetNode>("Containers//List");
          ret->registerModel<ListSizeNode>("Containers//List");
          ret->registerModel<ListClearNode>("Containers//List");
          ret->registerModel<ListEraseNode>("Containers//List");
          ret->registerModel<ListReserveNode>("Containers//List");
          ret->registerModel<ListDeclareNode>("Containers//List");


          ret->REGISTER_TYPE_CONVERTER(Decimal, Integer);
          ret->REGISTER_TYPE_CONVERTER(Decimal, Boolean);
          ret->REGISTER_TYPE_CONVERTER(Decimal, UnsignedInteger);

          ret->REGISTER_TYPE_CONVERTER(Integer, Decimal);
          ret->REGISTER_TYPE_CONVERTER(Integer, Boolean);
          ret->REGISTER_TYPE_CONVERTER(Integer, UnsignedInteger);

          ret->REGISTER_TYPE_CONVERTER(Boolean, Decimal);
          ret->REGISTER_TYPE_CONVERTER(Boolean, Integer);
          ret->REGISTER_TYPE_CONVERTER(Boolean, UnsignedInteger);

          ret->REGISTER_TYPE_CONVERTER(Integer, String);
          ret->REGISTER_TYPE_CONVERTER(Decimal, String);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, String);

          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Decimal);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Integer);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Boolean);

          ret->REGISTER_TYPE_CONVERTER(Integer, Basic);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Basic);
          ret->REGISTER_TYPE_CONVERTER(Decimal, Basic);
          ret->REGISTER_TYPE_CONVERTER(String, Basic);

          // Color
          ret->REGISTER_TYPE_CONVERTER(Integer, Color);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Color);
          ret->REGISTER_TYPE_CONVERTER(Decimal, Color);
          ret->REGISTER_TYPE_CONVERTER(Color, String);
          ret->REGISTER_TYPE_CONVERTER(Color, Vector4D);
          ret->REGISTER_TYPE_CONVERTER(Vector4D, Color);

          return ret;
        }


        static void setStyle()
        {
          ConnectionStyle::setConnectionStyle(
              R"(
              {
              "FlowViewStyle": {
                "BackgroundColor": [53, 53, 53],
                "FineGridColor": [60, 60, 60],
                "CoarseGridColor": [25, 25, 25]
              },
              "NodeStyle": {
                "NormalBoundaryColor": [255, 255, 255],
                "SelectedBoundaryColor": [255, 165, 0],
                "GradientColor0": "gray",
                "GradientColor1": [80, 80, 80],
                "GradientColor2": [64, 64, 64],
                "GradientColor3": [58, 58, 58],
                "ShadowColor": [20, 20, 20],
                "FontColor" : "white",
                "FontColorFaded" : "gray",
                "ConnectionPointColor": [169, 169, 169],
                "FilledConnectionPointColor": "cyan",
                "ErrorColor": "red",
                "WarningColor": [128, 128, 0],

                "PenWidth": 1.0,
                "HoveredPenWidth": 1.5,

                "ConnectionPointDiameter": 8.0,

                "Opacity": 0.8
              },
              "ConnectionStyle": {
                "ConstructionColor": "gray",
                "NormalColor": "darkcyan",
                "SelectedColor": [100, 100, 100],
                "SelectedHaloColor": "orange",
                "HoveredColor": "lightcyan",

                "LineWidth": 3.0,
                "ConstructionLineWidth": 2.0,
                "PointDiameter": 10.0,

                "UseDataDefinedColors": true
              }
            }
            )");
        }
    }
}

#endif //NOGGIT_NODEREGISTRY_HPP
