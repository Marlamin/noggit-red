// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "AddObjectInstance.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

#include <typeinfo>

using namespace noggit::Red::NodeEditor::Nodes;

AddObjectInstanceNode::AddObjectInstanceNode()
: ContextLogicNodeBase()
{
  setName("Object :: AddObjectInstance");
  setCaption("Object :: AddObjectInstance");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<StringData>(PortType::In, "Path<String>", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<Vector3DData>(PortType::In, "Rotation<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Scale<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ObjectInstanceData>(PortType::Out, "ObjectInstance", true);
}

void AddObjectInstanceNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto path_data = defaultPortData<StringData>(PortType::In, 1);
  std::string const& path = path_data->value();

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 2);
  glm::vec3 const& pos = pos_data->value();

  auto dir_data = defaultPortData<Vector3DData>(PortType::In, 3);
  glm::vec3 const& dir = dir_data->value();

  float scale = defaultPortData<DecimalData>(PortType::In, 4)->value();

  if (path.empty())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: path is empty.");
    return;
  }

  SceneObject* obj;

  if (QString(path.c_str()).endsWith(".m2", Qt::CaseInsensitive))
  {
    noggit::object_paste_params paste_params;
    obj = world->addM2AndGetInstance(path, {pos.x, pos.y, pos.z}, scale, {math::degrees(dir.x)._, math::degrees(dir.y)._, math::degrees(dir.z)._ }, &paste_params);
  }
  else if (QString(path.c_str()).endsWith(".wmo", Qt::CaseInsensitive))
  {
    obj = world->addWMOAndGetInstance(path, {pos.x, pos.y, pos.z}, {math::degrees(dir.x)._, math::degrees(dir.y)._, math::degrees(dir.z)._ });
  }
  else
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: path is not a valid .m2 or .wmo file.");
    return;
  }

  NOGGIT_CUR_ACTION->registerObjectAdded(obj);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ObjectInstanceData>(obj);
  _node->onDataUpdated(1);

}

