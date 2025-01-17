// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CONTEXT_HPP
#define NOGGIT_CONTEXT_HPP

#include <noggit/ui/tools/ViewportManager/ViewportManager.hpp>
#include <external/tsl/robin_map.h>

#include <QJsonDocument>

namespace QtNodes
{
  class DataModelRegistry;
  class NodeData;
}

class World;

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class Context;
        class NodeScene;

        using VariableMap = tsl::robin_map<std::string, std::pair<std::string, std::shared_ptr<QtNodes::NodeData>>>;

        enum NodeExecutionContext
        {
            MAP_VIEW,
            PRESET_EDITOR,
            ASSET_BROWSER
        };

        class Context : QObject
        {
            Q_OBJECT

        public:
            Context(NodeExecutionContext context_type, QObject* parent = nullptr);

            NodeScene* getScene(QString const& path, QObject* parent = nullptr);
            NodeExecutionContext getContextType() const;
            void makeCurrent();
            Nodes::VariableMap* getVariableMap();
            World* getWorld();
            ViewportManager::Viewport* getViewport();

            void setWorld(World* world);
            void setViewport(ViewportManager::Viewport* viewport);

        private:
            World* _world;
            ViewportManager::Viewport* _viewport;
            tsl::robin_map<std::string, QJsonDocument> _scene_cache;
            NodeExecutionContext _context_type;
            VariableMap _variable_map;
        };

        extern Context* gCurrentContext;
        extern std::shared_ptr<QtNodes::DataModelRegistry> gDataModelRegistry;
    }
}

#endif //NOGGIT_CONTEXT_HPP
