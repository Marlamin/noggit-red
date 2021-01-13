// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseGeneratorBase.hpp"


#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseGeneratorBase::NoiseGeneratorBase()
: BaseNode()
{
}


QJsonObject NoiseGeneratorBase::save() const
{
  QJsonObject json_obj = BaseNode::save();

  for (int i = 0; i < _in_ports.size(); ++i)
  {
    defaultWidgetToJson(PortType::In, i, json_obj, _in_ports[i].caption);
  }

  return json_obj;
}

void NoiseGeneratorBase::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  for (int i = 0; i < _in_ports.size(); ++i)
  {
    defaultWidgetFromJson(PortType::In, i, json_obj, _in_ports[i].caption);
  }

}

