// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NODEEDITOR_HPP
#define NOGGIT_NODEEDITOR_HPP


#include <QMainWindow>


class QFileSystemModel;
class QSortFilterProxyModel;
class QWidget;

namespace Ui
{
  class NodeEditor;
}

namespace Noggit
{
  namespace Ui::Tools
  {
    class PreviewRenderer;
  }

  namespace Ui::Tools::NodeEditor::Ui
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
