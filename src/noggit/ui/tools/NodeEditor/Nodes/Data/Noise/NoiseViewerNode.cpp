// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseViewerNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/LogicBranch.hpp>

#include <external/NodeEditor/include/nodes/Node>

#include <external/libnoise/noiseutils/noiseutils.h>

#include <QPushButton>
#include <QLabel>

#include <QSize>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

NoiseViewerNode::NoiseViewerNode()
: BaseNode()
{
  setName("Noise :: Viewer");
  setCaption("Noise :: Viewer");
  _validation_state = NodeValidationState::Valid;

  _image = new QLabel(&_embedded_widget);

  QPixmap pixmap(256, 256);
  pixmap.fill(Qt::black);

  _image->setPixmap(pixmap);
  addWidgetTop(_image);

  _update_btn = new QPushButton("Update", &_embedded_widget);
  addWidgetTop(_update_btn);

  QPushButton::connect(_update_btn, &QPushButton::clicked,
                       [=]()
                       {
                           LogicBranch::executeNodeLeaves(_node, _node);
                           compute();
                       }
  );

  addPort<NoiseData>(PortType::In, "Noise", true);
  addPort<NoiseData>(PortType::Out, "Noise", true);

}
void NoiseViewerNode::compute()
{
  noise::module::Module* noise_module = static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value();

  utils::NoiseMap noise_map;
  utils::NoiseMapBuilderPlane map_builder;
  map_builder.SetSourceModule(*noise_module);
  map_builder.SetDestNoiseMap(noise_map);
  map_builder.SetDestSize(256, 256);
  map_builder.SetBounds(0, 10, 0, 10);
  map_builder.Build();

  QImage image = QImage(QSize(256, 256),  QImage::Format_RGBA8888);

  for (int x = 0; x < static_cast<int>(256); ++x)
  {
    for (int y = 0; y < static_cast<int>(256); ++y)
    {
      float value = noise_map.GetValue(x, y);
      image.setPixelColor(x, y, QColor::fromRgbF(value, value, value, 1.0f));
    }
  }

  _image->setPixmap(QPixmap::fromImage(image));

  _out_ports[0].out_value = _in_ports[0].in_value.lock();
  _node->onDataUpdated(0);

}

NodeValidationState NoiseViewerNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
