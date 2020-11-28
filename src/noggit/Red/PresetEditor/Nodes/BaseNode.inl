#ifndef NOGGIT_BASENODE_INL
#define NOGGIT_BASENODE_INL

#include "BaseNode.hpp"

template<typename T>
void noggit::Red::PresetEditor::Nodes::BaseNode::addPort(PortType port_type, const QString &caption, bool caption_visible)
{
  if (port_type == PortType::In)
  {
    auto& port = _in_ports.emplace_back(caption, caption_visible);
    port.data_type = std::make_unique<T>();
  }
  else if (port_type == PortType::Out)
  {
    auto& port = _out_ports.emplace_back(caption, caption_visible);
    port.data_type = std::make_unique<T>();
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }
}

#endif // NOGGIT_BASENODE_INL