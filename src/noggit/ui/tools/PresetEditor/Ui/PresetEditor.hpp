#ifndef NOGGIT_PRESETEDITOR_HPP
#define NOGGIT_PRESETEDITOR_HPP

#include <ui_PresetEditor.h>
#include <ui_PresetEditorOverlay.h>

#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>
#include <noggit/ui/tools/PresetEditor/ModelView.hpp>

#include <QWidget>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

namespace Noggit
{
    namespace Ui::Tools::PresetEditor::Ui
    {
      class PresetEditorWidget : public QMainWindow
      {
      public:
        PresetEditorWidget(std::shared_ptr<Project::NoggitProject> project, QWidget* parent = nullptr);
        ~PresetEditorWidget();

      private:
        void setupConnectsCommon();
        std::shared_ptr<Project::NoggitProject> _project;
        ::Ui::PresetEditor* ui;
        ::Ui::PresetEditorOverlay* viewport_overlay_ui;
        QFileSystemModel* _model;
        QSortFilterProxyModel* _sort_model;
        PreviewRenderer* _preview_renderer;


      };
    }
}

#endif //NOGGIT_PRESETEDITOR_HPP
