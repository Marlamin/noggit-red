#ifndef NOGGIT_ASSETBROWSER_HPP
#define NOGGIT_ASSETBROWSER_HPP

#include <ui_AssetBrowser.h>
#include <ui_AssetBrowserOverlay.h>
#include <noggit/Red/AssetBrowser/Ui/Model/TreeManager.hpp>
#include <noggit/Red/PreviewRenderer/PreviewRenderer.hpp>

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QMainWindow>


namespace noggit
{
  namespace Red::AssetBrowser::Ui
  {
    class AssetBrowserWidget : public QMainWindow
    {
    public:
      AssetBrowserWidget(QWidget* parent = nullptr);
      ~AssetBrowserWidget();

    private:
      ::Ui::AssetBrowser* ui;
      ::Ui::AssetBrowserOverlay* viewport_overlay_ui;
      QStandardItemModel* _model;
      QSortFilterProxyModel* _sort_model;
      PreviewRenderer* _preview_renderer;
      QRegularExpression _wmo_group_and_lod_regex;

      void updateModelData();
      void recurseDirectory(Model::TreeManager& tree_mgr, const QString& s_dir, const QString& project_dir);

    protected:
      void keyPressEvent(QKeyEvent* event) override;

      void setupConnectsCommon();

    };
  }
}


#endif //NOGGIT_ASSETBROWSER_HPP
