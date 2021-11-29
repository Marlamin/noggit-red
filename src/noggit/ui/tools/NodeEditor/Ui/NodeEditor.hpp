// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NODEEDITOR_HPP
#define NOGGIT_NODEEDITOR_HPP

#include <ui_NodeEditor.h>
#include "../NodeRegistry.hpp"

#include <QWidget>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>

namespace noggit
{
    namespace ui::tools::NodeEditor::Ui
    {
        class NodeEditorWidget : public QMainWindow
        {
        public:
            explicit NodeEditorWidget(QWidget* parent = nullptr);
            ~NodeEditorWidget() override;

            void loadScene(QString const& filepath);

        private:

            ::Ui::NodeEditor* ui;

            QFileSystemModel* _model;
            QSortFilterProxyModel* _sort_model;
            PreviewRenderer* _preview_renderer;
        };
    }
}

#endif //NOGGIT_NODEEDITOR_HPP
