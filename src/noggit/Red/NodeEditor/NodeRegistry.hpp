// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NODEREGISTRY_HPP
#define NOGGIT_NODEREGISTRY_HPP

#include <external/NodeEditor/include/nodes/NodeData>
#include <external/NodeEditor/include/nodes/FlowScene>
#include <external/NodeEditor/include/nodes/FlowView>
#include <external/NodeEditor/include/nodes/ConnectionStyle>
#include <external/NodeEditor/include/nodes/TypeConverter>

#include <noggit/Red/NodeEditor/Nodes/Math/MathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/MathUnaryNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/Math/Color/ColorMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Color/ColorToRGBANode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Color/RGBAtoColorNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/Math/Vector/VectorMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Vector/VectorScalarMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Vector/Vector2DToXYNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Vector/Vector3DToXYZNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Vector/Vector4DToXYZWNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Vector/XYtoVector2D.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Vector/XYZtoVector3DNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Vector/XYZWtoVector4DNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/Math/Matrix/MatrixMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Matrix/MatrixDecomposeNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Matrix/MatrixUnaryMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Matrix/MatrixTransformNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Matrix/MatrixRotateQuaternionNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/Logic/ConditionNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicIfNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicBeginNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicBreakNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicContinueNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicReturnNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicReturnNoDataNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Functions/PrintNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicForLoopNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicWhileLoopNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicChainNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Logic/LogicProcedureNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/DataConstantNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/Containers/List/DataListNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Containers/List/ListAddNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Containers/List/ListGetNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Containers/List/ListSizeNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Containers/List/ListClearNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Containers/List/ListEraseNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Containers/List/ListReserveNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Containers/List/ListDeclareNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/Data/GetVariableNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/GetVariableLazyNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/SetVariableNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/DeleteVariableNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Math/Matrix/MatrixNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/Data/Image/LoadImageNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageInfoNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageSaveNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageCreateNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageTranslateNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageScaleNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageRotateNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageSetPixelNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageGetPixelNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageToGrayscaleNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageFillNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageInvertNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageMirrorNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageResizeNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Image/ImageGaussianBlurNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoisePerlinNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseToImageNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseVoronoiNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseBillowNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseCheckerboardNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseConstValueNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseCylindersNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseSpheresNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseRidgedMultiNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseMathNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseAbsNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseBlendNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseCacheNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseClampNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseDisplaceNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseExponentNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseInvertNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseScaleBiasNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseSelectNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseTransformPointNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseTurbulenceNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseCurveNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/Data/Noise/NoiseTerraceNode.hpp>

#include <noggit/Red/NodeEditor/Nodes/BaseNode.hpp>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericTypeConverter.hpp>
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
          ret->registerModel<MatrixNode>("Math//Matrix");
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

          // Image
          ret->registerModel<LoadImageNode>("Data//Image");
          ret->registerModel<ImageInfoNode>("Data//Image");
          ret->registerModel<ImageSaveNode>("Data//Image");
          ret->registerModel<ImageCreateNode>("Data//Image");
          ret->registerModel<ImageTranslateNode>("Data//Image");
          ret->registerModel<ImageScaleNode>("Data//Image");
          ret->registerModel<ImageRotateNode>("Data//Image");
          ret->registerModel<ImageInvertNode>("Data//Image");
          ret->registerModel<ImageFillNode>("Data//Image");
          ret->registerModel<ImageResizeNode>("Data//Image");
          ret->registerModel<ImageGetPixelNode>("Data//Image");
          ret->registerModel<ImageSetPixelNode>("Data//Image");
          ret->registerModel<ImageToGrayscaleNode>("Data//Image");
          ret->registerModel<ImageMirrorNode>("Data//Image");
          ret->registerModel<ImageGaussianBlurNode>("Data//Image");

          // Noise
          ret->registerModel<NoisePerlinNode>("Data//Noise//Generators");
          ret->registerModel<NoiseBillowNode>("Data//Noise//Generators");
          ret->registerModel<NoiseVoronoiNode>("Data//Noise//Generators");
          ret->registerModel<NoiseRidgedMultiNode>("Data//Noise//Generators");
          ret->registerModel<NoiseSpheresNode>("Data//Noise//Generators");
          ret->registerModel<NoiseCylindersNode>("Data//Noise//Generators");
          ret->registerModel<NoiseConstValueNode>("Data//Noise//Generators");
          ret->registerModel<NoiseCheckerboardNode>("Data//Noise//Generators");
          ret->registerModel<NoiseMathNode>("Data//Noise");
          ret->registerModel<NoiseToImageNode>("Data//Noise");
          ret->registerModel<NoiseCacheNode>("Data//Noise");
          ret->registerModel<NoiseAbsNode>("Data//Noise//Modifiers");
          ret->registerModel<NoiseClampNode>("Data//Noise//Modifiers");
          ret->registerModel<NoiseExponentNode>("Data//Noise//Modifiers");
          ret->registerModel<NoiseInvertNode>("Data//Noise//Modifiers");
          ret->registerModel<NoiseScaleBiasNode>("Data//Noise//Modifiers");
          ret->registerModel<NoiseCurveNode>("Data//Noise//Modifiers");
          ret->registerModel<NoiseTerraceNode>("Data//Noise//Modifiers");
          ret->registerModel<NoiseBlendNode>("Data//Noise//Selectors");
          ret->registerModel<NoiseSelectNode>("Data//Noise//Selectors");
          ret->registerModel<NoiseDisplaceNode>("Data//Noise//Transform");
          ret->registerModel<NoiseTransformPointNode>("Data//Noise//Transform");
          ret->registerModel<NoiseTurbulenceNode>("Data//Noise//Transform");

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
