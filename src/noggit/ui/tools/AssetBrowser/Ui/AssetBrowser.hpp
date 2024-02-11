#ifndef NOGGIT_ASSETBROWSER_HPP
#define NOGGIT_ASSETBROWSER_HPP

#include <ui_AssetBrowser.h>
#include <ui_AssetBrowserOverlay.h>
#include <noggit/ui/tools/AssetBrowser/Ui/Model/TreeManager.hpp>
#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QMainWindow>

class MapView;

// custom model that makes the searched children expend, credit to https://stackoverflow.com/questions/56781145/expand-specific-items-in-a-treeview-during-filtering
class NoggitExpendableFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    QList<QModelIndex> findIndices() const
    {
        QList<QModelIndex> ret;
        for (auto iter = 0; iter < rowCount(); iter++)
        {
            auto childIndex = index(iter, 0, QModelIndex());
            ret << recursivelyFindIndices(childIndex);
        }
        return ret;
    }


    bool rowAccepted(int source_row, const QModelIndex& source_parent) const
    {
        return filterAcceptsRow(source_row, source_parent);
    }
private:
    QList<QModelIndex> recursivelyFindIndices(const QModelIndex& ind) const
    {
        QList<QModelIndex> ret;
        if (rowAccepted(ind.row(), ind.parent()))
        {
            ret << ind;
        }
        else
        {
            for (auto iter = 0; iter < rowCount(ind); iter++)
            {
                ret << recursivelyFindIndices(index(iter, 0, ind));
            }
        }
        return ret;
    }
};


namespace Noggit::Ui::Tools::AssetBrowser
{
    enum class asset_browse_mode
    {
        world, // any wmo and m2 in world folder. exclude detail doodads?
        detail_doodads, // "world/nodxt/detail/
        skybox, // environements folder. used by LightSkybox
        creatures,
        characters,
        particles,
        cameras,
        items,
        spells,
        ALL
    };
}


namespace Noggit
{
  namespace Ui::Tools::AssetBrowser::Ui
  {
    class AssetBrowserWidget : public QMainWindow
    {
      Q_OBJECT
    public:
      AssetBrowserWidget(MapView* map_view, QWidget* parent = nullptr);
      ~AssetBrowserWidget();
      std::string const& getFilename() { return _selected_path; };

      void set_browse_mode(asset_browse_mode browse_mode);

      asset_browse_mode _browse_mode = asset_browse_mode::world;
    signals:
        void gl_data_unloaded();

    private:
      ::Ui::AssetBrowser* ui;
      ::Ui::AssetBrowserOverlay* viewport_overlay_ui;
      QStandardItemModel* _model;
      NoggitExpendableFilterProxyModel* _sort_model;
      PreviewRenderer* _preview_renderer;
      QRegularExpression _wmo_group_and_lod_regex;
      MapView* _map_view;
      std::string _selected_path;

      void updateModelData();
      void recurseDirectory(Model::TreeManager& tree_mgr, const QString& s_dir, const QString& project_dir);
      bool validateBrowseMode(QString wow_file_path);

    protected:
      void keyPressEvent(QKeyEvent* event) override;

      void setupConnectsCommon();

    };
  }
}


#endif //NOGGIT_ASSETBROWSER_HPP
