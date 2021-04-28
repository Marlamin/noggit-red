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

class MapView;

namespace noggit
{
  namespace Red::AssetBrowser::Ui
  {
    class AssetBrowserWidget : public QMainWindow
    {
      Q_OBJECT
    public:
      AssetBrowserWidget(MapView* map_view, QWidget* parent = nullptr);
      ~AssetBrowserWidget();
      std::string const& getFilename() { return _selected_path; };

    signals:
        void gl_data_unloaded();

    private:
      ::Ui::AssetBrowser* ui;
      ::Ui::AssetBrowserOverlay* viewport_overlay_ui;
      QStandardItemModel* _model;
      QSortFilterProxyModel* _sort_model;
      PreviewRenderer* _preview_renderer;
      QRegularExpression _wmo_group_and_lod_regex;
      MapView* _map_view;
      std::string _selected_path;

      void updateModelData();
      void recurseDirectory(Model::TreeManager& tree_mgr, const QString& s_dir, const QString& project_dir);

    protected:
      void keyPressEvent(QKeyEvent* event) override;

      void setupConnectsCommon();

    };
  }
}


#endif //NOGGIT_ASSETBROWSER_HPP
