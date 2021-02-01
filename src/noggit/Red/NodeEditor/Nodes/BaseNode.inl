#ifndef NOGGIT_BASENODE_INL
#define NOGGIT_BASENODE_INL

#include "BaseNode.hpp"
#include "DataTypes/GenericData.hpp"

template<typename T>
void noggit::Red::NodeEditor::Nodes::BaseNode::addPort(PortType port_type,
                                                              const QString &caption,
                                                              bool caption_visible,
                                                              ConnectionPolicy out_policy)
{
  // force one output for logic data
  ConnectionPolicy new_out_policy = out_policy;
  if constexpr (std::is_same<T, LogicData>::value)
  {
    new_out_policy = ConnectionPolicy::One;
  }

  if (port_type == PortType::In)
  {
    auto& port = _in_ports.emplace_back(caption, caption_visible);
    port.data_type = std::make_unique<T>();
  }
  else if (port_type == PortType::Out)
  {
    auto& port = _out_ports.emplace_back(caption, caption_visible);
    port.data_type = std::make_unique<T>();

    //assert(port.data_type->type().id == "logic" && out_policy == ConnectionPolicy::Many);
    port.connection_policy = new_out_policy;
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }

}

template<typename T>
void noggit::Red::NodeEditor::Nodes::BaseNode::addPort(PortType port_type,
                                                              PortIndex port_index,
                                                              const QString &caption,
                                                              bool caption_visible,
                                                              ConnectionPolicy out_policy)
{
  // force one output for logic data
  ConnectionPolicy new_out_policy = out_policy;
  if constexpr (std::is_same<T, LogicData>::value)
  {
    new_out_policy = ConnectionPolicy::One;
  }

  if (port_type == PortType::In)
  {
    auto port = _in_ports.emplace(_in_ports.begin() + port_index, caption, caption_visible);
    port->data_type = std::make_unique<T>();
  }
  else if (port_type == PortType::Out)
  {
    auto port = _out_ports.emplace(_out_ports.begin() + port_index, caption, caption_visible);
    port->data_type = std::make_unique<T>();

    assert(!(port->data_type->type().id == "logic" && out_policy == ConnectionPolicy::Many));
    port->connection_policy = new_out_policy;
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }
}

template<typename T>
void noggit::Red::NodeEditor::Nodes::BaseNode::addPortDynamic(PortType port_type, PortIndex port_index,
                                                                     const QString& caption, bool caption_visible,
                                                                     ConnectionPolicy out_policy)
{
  addPort<T>(port_type, port_index, caption, caption_visible, out_policy);
  Q_EMIT portAdded(port_type, port_index);
}

template<typename T>
void noggit::Red::NodeEditor::Nodes::BaseNode::addPortDefault(PortType port_type,
                                                                    const QString &caption,
                                                                    bool caption_visible,
                                                                    ConnectionPolicy out_policy)
{
  addPort<T>(port_type, caption, caption_visible, out_policy);

  if (port_type == PortType::In)
  {
    PortIndex index = _in_ports.size() - 1;
    addDefaultWidget(_in_ports[index].data_type->default_widget(&_embedded_widget), port_type, index);
  }
  else if (port_type == PortType::Out)
  {
    PortIndex index = _out_ports.size() - 1;
    addDefaultWidget(_out_ports[index].data_type->default_widget(&_embedded_widget), port_type, index);
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }

}

template<typename T>
void noggit::Red::NodeEditor::Nodes::BaseNode::addPortDefault(PortType port_type,
                                                                     PortIndex port_index,
                                                                     const QString &caption,
                                                                     bool caption_visible,
                                                                     ConnectionPolicy out_policy)
{
  addPort<T>(port_type, port_index, caption, caption_visible, out_policy);

  if (port_type == PortType::In)
  {
    addDefaultWidget(_in_ports[port_index].data_type->default_widget(&_embedded_widget), port_type, port_index);
  }
  else if (port_type == PortType::Out)
  {
    addDefaultWidget(_out_ports[port_index].data_type->default_widget(&_embedded_widget), port_type, port_index);
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }
}

template<typename T>
void noggit::Red::NodeEditor::Nodes::BaseNode::addPortDefaultDynamic(PortType port_type, PortIndex port_index,
                                                                            const QString& caption, bool caption_visible,
                                                                            ConnectionPolicy out_policy)
{
  addPortDefault<T>(port_type, port_index, caption, caption_visible, out_policy);
  Q_EMIT portAdded(port_type, port_index);
}

template <typename T>
std::shared_ptr<T> noggit::Red::NodeEditor::Nodes::BaseNode::defaultPortData(PortType port_type,
                                                                                    PortIndex port_index)
{
  if (port_type == PortType::In)
  {
    T* data_ptr = static_cast<T*>(_in_ports[port_index].in_value.lock().get());
    auto ret = data_ptr ? _in_ports[port_index].in_value.lock() : _in_ports[port_index].data_type->default_widget_data(_in_ports[port_index].default_widget);
    return std::static_pointer_cast<T>(ret);
  }
  else if (port_type == PortType::Out)
  {
    auto ret = _out_ports[port_index].data_type->default_widget_data(_out_ports[port_index].default_widget);
    return std::static_pointer_cast<T>(ret);
  }

  throw std::logic_error("Incorrect port type or port type None.");
}

#endif // NOGGIT_BASENODE_INL