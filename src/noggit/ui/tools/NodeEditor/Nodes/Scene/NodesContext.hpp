// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CONTEXT_HPP
#define NOGGIT_CONTEXT_HPP

#include <noggit/World.h>
#include <noggit/ui/tools/ViewportManager/ViewportManager.hpp>
#include "NodeScene.hpp"
#include <external/tsl/robin_map.h>

#include <QObject>
#include <QJsonDocument>

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        using VariableMap = tsl::robin_map<std::string, std::pair<std::string, std::shared_ptr<NodeData>>>;

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
            NodeExecutionContext getContextType() { return _context_type; };
            void makeCurrent();
            VariableMap* getVariableMap() { return &_variable_map; };
            World* getWorld() { return _world; };
            ViewportManager::Viewport* getViewport() { return _viewport; };

            void setWorld(World* world) { _world = world; };
            void setViewport(ViewportManager::Viewport* viewport) { _viewport = viewport; };

        private:
            World* _world;
            ViewportManager::Viewport* _viewport;
            tsl::robin_map<std::string, QJsonDocument> _scene_cache;
            NodeExecutionContext _context_type;
            VariableMap _variable_map;

        };

        extern Context* gCurrentContext;
        extern std::shared_ptr<DataModelRegistry> gDataModelRegistry;
    }
}

#endif //NOGGIT_CONTEXT_HPP
