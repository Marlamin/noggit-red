// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SaveJSONObject.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <QJsonDocument>
#include <QDir>
#include <QFileInfo>

using namespace noggit::Red::NodeEditor::Nodes;

SaveJSONObjectNode::SaveJSONObjectNode()
: LogicNodeBase()
{
  setName("JSON :: SaveJSONObjectNode");
  setCaption("JSON :: SaveJSONObject");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<StringData>(PortType::In, "Path<String>", true);
  addPortDefault<JSONData>(PortType::In, "JSONObject", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void SaveJSONObjectNode::compute()
{
  QJsonObject* json_obj = defaultPortData<JSONData>(PortType::In, 2)->value_ptr();
  std::string path = defaultPortData<StringData>(PortType::In, 1)->value();

  QJsonDocument json_doc = QJsonDocument(*json_obj);

  QDir path_folder = QFileInfo(path.c_str()).absoluteDir();
  if(!path_folder.exists())
  {
    path_folder.mkpath(".");
  }

  QFile file(path.c_str());

  if (path.empty() || !file.open(QIODevice::WriteOnly))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: saving image failed.");
    return;
  }

  file.write(json_doc.toJson());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState SaveJSONObjectNode::validate()
{

  if (!static_cast<JSONData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject SaveJSONObjectNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "path");

  return json_obj;
}

void SaveJSONObjectNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 1, json_obj, "path");
}

