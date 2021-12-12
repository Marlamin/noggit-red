// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "Context.hpp"
#include "../../NodeRegistry.hpp"

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

void Context::makeCurrent()
{
  gCurrentContext = this;
}

