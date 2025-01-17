#ifndef NOGGIT_PRESETEDITOR_HPP
#define NOGGIT_PRESETEDITOR_HPP

#include <QWidget>
#include <QMainWindow>

class QFileSystemModel;
class QSortFilterProxyModel;

namespace Ui
{
  class PresetEditor;
  class PresetEditorOverlay;
}

namespace Noggit
{
  namespace Project
  {
    class NoggitProject;
  }

  namespace Ui::Tools
  {
    class PreviewRenderer;

    namespace PresetEditor::Ui
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
}

#endif //NOGGIT_PRESETEDITOR_HPP
