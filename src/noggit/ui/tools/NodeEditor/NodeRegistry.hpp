// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NODEREGISTRY_HPP
#define NOGGIT_NODEREGISTRY_HPP

#include <external/NodeEditor/include/nodes/NodeData>
#include <external/NodeEditor/include/nodes/FlowScene>
#include <external/NodeEditor/include/nodes/FlowView>
#include <external/NodeEditor/include/nodes/ConnectionStyle>
#include <external/NodeEditor/include/nodes/FlowViewStyle>
#include <external/NodeEditor/include/nodes/NodeStyle>
#include <external/NodeEditor/include/nodes/TypeConverter>

// Mandatory nodes
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/ConditionNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicIfNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicBeginNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicBreakNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicContinueNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicReturnNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicReturnNoDataNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicForLoopNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicWhileLoopNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicChainNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Logic/LogicProcedureNode.hpp>

#ifndef DO_NOT_BUILD_NODES

#include <noggit/ui/tools/NodeEditor/Nodes/Functions/PrintNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/MathNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/MathUnaryNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Math/Color/ColorMathNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Color/ColorToRGBANode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Color/RGBAtoColorNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Math/Vector/VectorMathNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Vector/VectorScalarMathNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Vector/Vector2DToXYNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Vector/Vector3DToXYZNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Vector/Vector4DToXYZWNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Vector/XYtoVector2D.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Vector/XYZtoVector3DNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Vector/XYZWtoVector4DNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Math/Matrix/MatrixMathNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Matrix/MatrixDecomposeNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Matrix/MatrixUnaryMathNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Matrix/MatrixTransformNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Math/Matrix/MatrixRotateQuaternionNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Data/DataConstantNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Containers/List/DataListNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/List/ListAddNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/List/ListGetNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/List/ListSizeNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/List/ListClearNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/List/ListEraseNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/List/ListReserveNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/List/ListDeclareNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/CreateJSONObject.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/GetJSONValue.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/JSONObjectInfo.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/LoadJSONObject.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/SaveJSONObject.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/SetJSONValue.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/CreateJSONArray.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/JSONArrayGetValue.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/JSONArrayInfo.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/JSONArrayInsertValue.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Containers/JSON/JSONArrayPush.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Data/GetVariableNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/GetVariableLazyNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/SetVariableNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/DeleteVariableNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Data/String/StringEndsWithNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/String/StringSizeNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/String/StringConcatenateNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/String/StringEqual.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Math/Matrix/MatrixNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Data/Random/RandomDecimalNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Random/RandomDecimalRangeNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Random/RandomIntegerNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Random/RandomIntegerRangeNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Random/RandomSeedNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/LoadImageNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageInfoNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageSaveNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageCreateNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageTranslateNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageScaleNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageRotateNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageSetPixelNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageGetPixelNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageToGrayscaleNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageFillNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageInvertNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageMirrorNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageResizeNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageGaussianBlurNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageMaskRandomPointsNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageBlendOpenGLNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageSetRegionNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Image/ImageGetRegionNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoisePerlinNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseToImageNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseVoronoiNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseBillowNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseCheckerboardNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseConstValueNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseCylindersNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseSpheresNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseRidgedMultiNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseMathNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseAbsNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseBlendNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseCacheNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseClampNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseDisplaceNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseExponentNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseInvertNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseScaleBiasNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseSelectNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseTransformPointNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseTurbulenceNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseCurveNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseTerraceNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Data/Noise/NoiseViewerNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/RewiringPointNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainRaiseLowerNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainFlattenNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainBlurNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainClearHeight.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainClearVertexSelection.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainDeselectVertices.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainFlattenVertices.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainMoveSelectedVertices.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainOrientVerticesNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Terrain/TerrainSelectVertices.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Texturing/TexturingTilesetNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Texturing/TexturingPaintTextureNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Texturing/TexturingSprayTextureNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Texturing/TexturingClearTexturesAdtAtPosNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Texturing/TexturingRemoveTexDuplisAdtAtPosNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Texturing/TexturingSetAdtBaseTextureAtPosNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Texturing/TexturingSwapTextureAtPosNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Texturing/TexturingSwapTextureAtPosRadiusNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkGetAlphaLayer.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkSetAlphaLayer.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkGetHeightmap.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkGetHeightmapImage.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkSetHeightmap.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkSetHeightmapImage.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkInfoNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkClearHeight.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkClearShadows.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkSetAreaID.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkSwapTexture.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkEraseTextures.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkEraseUnusedTextures.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkCanPaintTexture.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkGetTextureByLayer.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkFindTextureNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkAddTextureNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkGetVertexColors.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkGetVertexColorsImage.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkSetVertexColors.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkSetVertexColorsImage.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkRecalculateNormals.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Chunk/ChunkAddDetailDoodads.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/GetChunk.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/GetChunkFromPos.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/GetChunksInRange.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/GetTile.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/GetTileChunks.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/GetTileFromPos.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/GetTilesInRange.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/HasTileAt.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Coordinates/HasTileAtPos.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/LoadedTiles/FixAllGapsNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Misc/WorldConstantsNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Shading/ShadingPaintColorNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Shading/ShadingPickColorNode.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/ReloadTileNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileGetMinMaxHeight.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileGetVertexNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileGetAlphaLayer.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileGetAlphaLayerTexture.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileGetHeightMapImage.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileSetHeightmapImage.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileSetAlphaLayer.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileGetObjectsUIDs.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileGetVertexColorsImage.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileSetVertexColorsImage.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Tile/TileRecalculateNormals.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Object/AddObjectInstance.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Object/ObjectInstanceInfo.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Object/ObjectInstanceSetPosition.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Object/ObjectInstanceSetRotation.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Object/ObjectInstanceSetScale.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Object/GetObjectInstanceByUID.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/AddObjectInstancesToSelectionRange.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/AddObjectInstanceToSelection.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/DeleteSelectedObjectInstances.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/DeselectObjectInstance.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/DeselectObjectInstanceByUID.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/GetLastSelectedObjectInstance.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/GetSelectedObjectInstances.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/IsObjectInstanceSelected.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/IsObjectInstanceSelectedUID.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/MoveSelectedObjectInstances.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/ResetSelection.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/RotateSelectedObjectInstances.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/ScaleSelectedObjectInstances.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/SelectionInfo.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/SetCurrentSelection.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/SetSelectedObjectInstancesPosition.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/SetSelectedObjectInstancesRotation.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Selection/SnapSelectedObjectInstancesToGround.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Liquid/CropWaterAdtAtPos.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Liquid/GetWaterType.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Liquid/SetWaterType.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Liquid/PaintLiquid.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/World/Holes/SetHole.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/World/Holes/SetHoleADTAtPos.hpp>

#endif

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericTypeConverter.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodeScene.hpp>

#include <QSettings>
#include <QDir>
#include <QByteArray>

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;
using QtNodes::ConnectionStyle;
using QtNodes::FlowViewStyle;
using QtNodes::NodeStyle;
using QtNodes::TypeConverter;
using QtNodes::TypeConverterId;

#define REGISTER_TYPE_CONVERTER(T_FROM, T_TO)                       \
        registerTypeConverter(std::make_pair(T_FROM##Data().type(), \
        T_TO##Data().type()),                                       \
        TypeConverter{T_FROM##To##T_TO##TypeConverter()})

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {

        static std::shared_ptr<DataModelRegistry> registerDataModels()
        {
          auto ret = std::make_shared<DataModelRegistry>();

          // Logic (this nodes are necessary to build)
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

#ifndef DO_NOT_BUILD_NODES
          ret->registerModel<PrintNode>("Functions//Generic");
          ret->registerModel<RewiringPointNode>("");

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
          ret->registerModel<ImageMaskRandomPointsNode>("Data//Image");
          ret->registerModel<ImageBlendOpenGLNode>("Data//Image");
          ret->registerModel<ImageSetRegionNode>("Data//Image");
          ret->registerModel<ImageGetRegionNode>("Data//Image");

          // String
          ret->registerModel<StringEndsWithNode>("Data//String");
          ret->registerModel<StringSizeNode>("Data//String");
          ret->registerModel<StringConcatenateNode>("Data//String");
          ret->registerModel<StringEqualNode>("Data//String");


          // Random
          ret->registerModel<RandomDecimalNode>("Data//Random");
          ret->registerModel<RandomDecimalRangeNode>("Data//Random");
          ret->registerModel<RandomIntegerNode>("Data//Random");
          ret->registerModel<RandomIntegerRangeNode>("Data//Random");
          ret->registerModel<RandomSeedNode>("Data//Random");

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
          ret->registerModel<NoiseViewerNode>("Data//Noise");
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

          // List
          ret->registerModel<DataListNode>("Containers//List");
          ret->registerModel<ListAddNode>("Containers//List");
          ret->registerModel<ListGetNode>("Containers//List");
          ret->registerModel<ListSizeNode>("Containers//List");
          ret->registerModel<ListClearNode>("Containers//List");
          ret->registerModel<ListEraseNode>("Containers//List");
          ret->registerModel<ListReserveNode>("Containers//List");
          ret->registerModel<ListDeclareNode>("Containers//List");

          // JSON
          ret->registerModel<CreateJSONObjectNode>("Containers//JSON");
          ret->registerModel<GetJSONValueNode>("Containers//JSON");
          ret->registerModel<JSONObjectInfoNode>("Containers//JSON");
          ret->registerModel<LoadJSONObjectNode>("Containers//JSON");
          ret->registerModel<SaveJSONObjectNode>("Containers//JSON");
          ret->registerModel<SetJSONValueNode>("Containers//JSON");
          ret->registerModel<CreateJSONArrayNode>("Containers//JSON");
          ret->registerModel<JSONArrayGetValueNode>("Containers//JSON");
          ret->registerModel<JSONArrayInfoNode>("Containers//JSON");
          ret->registerModel<JSONArrayInsertValueNode>("Containers//JSON");
          ret->registerModel<JSONArrayPushNode>("Containers//JSON");

          // Actions
          ret->registerModel<TerrainRaiseLowerNode>("World//Terrain//");
          ret->registerModel<TerrainFlattenNode>("World//Terrain//");
          ret->registerModel<TerrainBlurNode>("World//Terrain//");
          ret->registerModel<TerrainClearHeightNode>("World//Terrain//");
          ret->registerModel<TerrainClearVertexSelectionNode>("World//Terrain//");
          ret->registerModel<TerrainDeselectVerticesNode>("World//Terrain//");
          ret->registerModel<TerrainFlattenVerticesNode>("World//Terrain//");
          ret->registerModel<TerrainMoveSelectedVerticesNode>("World//Terrain//");
          ret->registerModel<TerrainOrientVerticesNode>("World//Terrain//");
          ret->registerModel<TerrainSelectVerticesNode>("World//Terrain//");

          ret->registerModel<TexturingTilesetNode>("World//Texturing//");
          ret->registerModel<TexturingPaintTextureNode>("World//Texturing//");
          ret->registerModel<TexturingSprayTextureNode>("World//Texturing//");
          ret->registerModel<TexturingClearTexturesAdtAtPosNode>("World//Texturing//");
          ret->registerModel<TexturingRemoveTexDuplisAdtAtPosNode>("World//Texturing//");
          ret->registerModel<TexturingSetAdtBaseTextureAtPosNode>("World//Texturing//");
          ret->registerModel<TexturingSwapTextureAtPosNode>("World//Texturing//");
          ret->registerModel<TexturingSwapTextureAtPosRadiusNode>("World//Texturing//");

          ret->registerModel<ShadingPaintColorNode>("World//Shading//");
          ret->registerModel<ShadingPickColorNode>("World//Shading//");

          ret->registerModel<WorldConstantsNode>("World//Misc//");

          ret->registerModel<ChunkGetAlphaLayerNode>("World//Chunk//");
          ret->registerModel<ChunkSetAlphaLayerNode>("World//Chunk//");
          ret->registerModel<ChunkGetHeightmapNode>("World//Chunk//");
          ret->registerModel<ChunkSetHeightmapNode>("World//Chunk//");
          ret->registerModel<ChunkGetHeightmapImageNode>("World//Chunk//");
          ret->registerModel<ChunkSetHeightmapImageNode>("World//Chunk//");
          ret->registerModel<ChunkInfoNode>("World//Chunk//");
          ret->registerModel<ChunkClearHeightNode>("World//Chunk//");
          ret->registerModel<ChunkClearShadowsNode>("World//Chunk//");
          ret->registerModel<ChunkSetAreaIDNode>("World//Chunk//");
          ret->registerModel<ChunkSwapTextureNode>("World//Chunk//");
          ret->registerModel<ChunkEraseTexturesNode>("World//Chunk//");
          ret->registerModel<ChunkEraseUnusedTexturesNode>("World//Chunk//");
          ret->registerModel<ChunkCanPaintTextureNode>("World//Chunk//");
          ret->registerModel<ChunkGetTextureByLayerNode>("World//Chunk//");
          ret->registerModel<ChunkFindTextureNode>("World//Chunk//");
          ret->registerModel<ChunkAddTextureNode>("World//Chunk//");
          ret->registerModel<ChunkGetVertexColorsNode>("World//Chunk//");
          ret->registerModel<ChunkGetVertexColorsImageNode>("World//Chunk//");
          ret->registerModel<ChunkSetVertexColorsNode>("World//Chunk//");
          ret->registerModel<ChunkSetVertexColorsImageNode>("World//Chunk//");
          ret->registerModel<ChunkRecalculateNormalsNode>("World//Chunk//");
          ret->registerModel<ChunkAddDetailDoodads>("World//Chunk//");

          ret->registerModel<ReloadTileNode>("World//Tile//");
          ret->registerModel<TileGetMinMaxHeightNode>("World//Tile//");
          ret->registerModel<TileGetVertexNode>("World//Tile//");
          ret->registerModel<TileGetAlphaLayerNode>("World//Tile//");
          ret->registerModel<TileGetAlphaLayerTextureNode>("World//Tile//");
          ret->registerModel<TileGetHeightmapImageNode>("World//Tile//");
          ret->registerModel<TileSetHeightmapImageNode>("World//Tile//");
          ret->registerModel<TileSetAlphaLayerNode>("World//Tile//");
          ret->registerModel<TileGetObjectUIDsNode>("World//Tile//");
          ret->registerModel<TileSetVertexColorsImageNode>("World//Tile//");
          ret->registerModel<TileGetVertexColorsImageNode>("World//Tile//");
          ret->registerModel<TileRecalculateNormalsNode>("World//Tile//");

          ret->registerModel<FixAllGapsNode>("World//Loaded Tiles//");

          ret->registerModel<GetChunkNode>("World//Coordinates//");
          ret->registerModel<GetChunkFromPosNode>("World//Coordinates//");
          ret->registerModel<GetChunksInRangeNode>("World//Coordinates//");
          ret->registerModel<GetTileNode>("World//Coordinates//");
          ret->registerModel<GetTileChunksNode>("World//Coordinates//");
          ret->registerModel<GetTileFromPosNode>("World//Coordinates//");
          ret->registerModel<GetTilesInRangeNode>("World//Coordinates//");
          ret->registerModel<HasTileAtNode>("World//Coordinates//");
          ret->registerModel<HasTileAtPosNode>("World//Coordinates//");

          ret->registerModel<AddObjectInstanceNode>("World//Object//");
          ret->registerModel<ObjectInstanceInfoNode>("World//Object//");
          ret->registerModel<ObjectInstanceSetPositionNode>("World//Object//");
          ret->registerModel<ObjectInstanceSetRotationNode>("World//Object//");
          ret->registerModel<ObjectInstanceSetScaleNode>("World//Object//");
          ret->registerModel<GetObjectInstanceByUIDNode>("World//Object//");

          ret->registerModel<AddObjectInstancesToSelectionRangeNode>("World//Selection//");
          ret->registerModel<AddObjectInstanceToSelectionNode>("World//Selection//");
          ret->registerModel<DeleteSelectedObjectInstancesNode>("World//Selection//");
          ret->registerModel<DeselectObjectInstanceNode>("World//Selection//");
          ret->registerModel<DeselectObjectInstanceByUIDNode>("World//Selection//");
          ret->registerModel<GetLastSelectedObjectInstanceNode>("World//Selection//");
          ret->registerModel<GetSelectedObjectInstancesNode>("World//Selection//");
          ret->registerModel<IsObjectInstanceSelectedNode>("World//Selection//");
          ret->registerModel<IsObjectInstanceSelectedUIDNode>("World//Selection//");
          ret->registerModel<MoveSelectedObjectInstancesNode>("World//Selection//");
          ret->registerModel<ResetSelectionNode>("World//Selection//");
          ret->registerModel<RotateSelectedObjectInstancesNode>("World//Selection//");
          ret->registerModel<ScaleSelectedObjectInstancesNode>("World//Selection//");
          ret->registerModel<SelectionInfoNode>("World//Selection//");
          ret->registerModel<SetCurrentSelectionNode>("World//Selection//");
          ret->registerModel<SetSelectedObjectInstancesPositionNode>("World//Selection//");
          ret->registerModel<SetSelectedObjectInstancesRotationNode>("World//Selection//");
          ret->registerModel<SnapSelectedObjectInstancesToGroundNode>("World//Selection//");

          ret->registerModel<PaintLiquidNode>("World//Liquid//");
          ret->registerModel<SetWaterTypeNode>("World//Liquid//");
          ret->registerModel<GetWaterTypeNode>("World//Liquid//");
          ret->registerModel<CropWaterADTAtPosNode>("World//Liquid//");

          ret->registerModel<SetHoleNode>("World//Holes//");
          ret->registerModel<SetHoleADTAtPosNode>("World//Holes//");


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

          // Color
          ret->REGISTER_TYPE_CONVERTER(Integer, Color);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Color);
          ret->REGISTER_TYPE_CONVERTER(Decimal, Color);
          ret->REGISTER_TYPE_CONVERTER(Color, String);
          ret->REGISTER_TYPE_CONVERTER(Color, Vector4D);
          ret->REGISTER_TYPE_CONVERTER(Vector4D, Color);

          // Json
          ret->REGISTER_TYPE_CONVERTER(Integer, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(Decimal, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(String, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(Color, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(Vector2D, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(Vector3D, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(Vector4D, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(Matrix4x4, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(Quaternion, JSONValue);
          ret->REGISTER_TYPE_CONVERTER(JSON, JSONValue);

          // Vector
          ret->REGISTER_TYPE_CONVERTER(Integer, Vector2D);
          ret->REGISTER_TYPE_CONVERTER(Integer, Vector3D);
          ret->REGISTER_TYPE_CONVERTER(Integer, Vector4D);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Vector2D);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Vector3D);
          ret->REGISTER_TYPE_CONVERTER(UnsignedInteger, Vector4D);
          ret->REGISTER_TYPE_CONVERTER(Decimal, Vector2D);
          ret->REGISTER_TYPE_CONVERTER(Decimal, Vector3D);
          ret->REGISTER_TYPE_CONVERTER(Decimal, Vector4D);

#endif

          return ret;
        }


        static void setStyle()
        {
          QSettings settings;
          QString theme = settings.value("theme", 1).toString();
          QDir theme_dir = QDir("./themes/");

          if (theme != "System" && theme_dir.exists() && QDir(theme_dir.path() + "/" + theme).exists("nodes_theme.json"))
          {
            QFile json_file = QFile(QDir(theme_dir.path() + "/" + theme).filePath("nodes_theme.json"));
            json_file.open(QIODevice::ReadOnly);

            QByteArray save_data = json_file.readAll();

            ConnectionStyle::setConnectionStyle(save_data);
            FlowViewStyle::setStyle(save_data);
            NodeStyle::setNodeStyle(save_data);
          }
        }
    }
}

#endif //NOGGIT_NODEREGISTRY_HPP
