// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NodesContext.hpp"
#include "NodeScene.hpp"
#include "../../NodeRegistry.hpp"

#include <noggit/World.h>

#include <QDir>
#include <QFileInfo>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

Context* Noggit::Ui::Tools::NodeEditor::Nodes::gCurrentContext = new Context(NodeExecutionContext::MAP_VIEW);
std::shared_ptr<DataModelRegistry> Noggit::Ui::Tools::NodeEditor::Nodes::gDataModelRegistry;

Context::Context(NodeExecutionContext context_type, QObject* parent)
: QObject(parent)
, _context_type(context_type)
{

}

NodeScene* Context::getScene(const QString& path, QObject* parent)
{
  if (!gDataModelRegistry)
    gDataModelRegistry = registerDataModels();

  auto it = _scene_cache.find(path.toStdString());

  if (it != _scene_cache.end())
  {
    auto document = it->second;

    auto scene = new NodeScene(gDataModelRegistry, parent);
    scene->loadFromMemory(document.object());

    return scene;
  }

  auto abs_path = QDir("./scripts/").absoluteFilePath(path);

  QFileInfo check_file(abs_path);

  if (!check_file.exists() || check_file.isDir())
    return nullptr;

  auto scene = new NodeScene(gDataModelRegistry, parent);

  scene->load(abs_path);

  auto document = scene->saveToJson();

  _scene_cache[path.toStdString()] = document;

  return scene;

}

NodeExecutionContext Context::getContextType() const
{
  return _context_type;
}

void Context::makeCurrent()
{
  gCurrentContext = this;
}

Noggit::Ui::Tools::NodeEditor::Nodes::VariableMap* Context::getVariableMap()
{
  return &_variable_map;
}

World* Context::getWorld()
{
  return _world;
}

Noggit::Ui::Tools::ViewportManager::Viewport* Context::getViewport()
{
  return _viewport;
}

void Context::setWorld(World* world)
{
  _world = world;
}

void Context::setViewport(ViewportManager::Viewport* viewport)
{
  _viewport = viewport;
}

