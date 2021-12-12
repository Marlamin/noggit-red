// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LoadJSONObject.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <QFile>
#include <QJsonDocument>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

LoadJSONObjectNode::LoadJSONObjectNode()
: LogicNodeBase()
{
  setName("JSON :: LoadJSONObject");
  setCaption("JSON :: LoadJSONObject");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<StringData>(PortType::In, "Path<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<JSONData>(PortType::Out, "JSONObject", true);
}

void LoadJSONObjectNode::compute()
{
  std::string path = defaultPortData<StringData>(PortType::In, 1)->value();

  QFile file(path.c_str());

  if (path.empty() || !file.open(QIODevice::ReadOnly))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: saving image failed.");
    return;
  }

  QByteArray data = file.readAll();

  QJsonDocument json_doc = QJsonDocument::fromJson(data);

  if (!json_doc.isObject())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: loaded json document does not have a top level object.");
    return;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<JSONData>(json_doc.object());
  _node->onDataUpdated(1);

}


QJsonObject LoadJSONObjectNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "path");

  return json_obj;
}

void LoadJSONObjectNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 1, json_obj, "path");
}

