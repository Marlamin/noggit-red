#ifndef NOGGIT_ASSETBROWSER_HPP
#define NOGGIT_ASSETBROWSER_HPP

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QMainWindow>
#include <QMap>

namespace Ui
{
  class AssetBrowser;
  class AssetBrowserOverlay;
}

class MapView;

class QStandardItemModel;

// custom model that makes the searched children expend, credit to https://stackoverflow.com/questions/56781145/expand-specific-items-in-a-treeview-during-filtering
class NoggitExpendableFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    QList<QModelIndex> findIndices() const;

    bool rowAccepted(int source_row, const QModelIndex& source_parent) const;
private:
    QList<QModelIndex> recursivelyFindIndices(const QModelIndex& ind) const;
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
  namespace Ui::Tools
  {
    class PreviewRenderer;
  namespace AssetBrowser::Ui
  {
    namespace Model
    {
      class TreeManager;
    }

    class AssetBrowserWidget : public QMainWindow
    {
      Q_OBJECT
    public:
      AssetBrowserWidget(MapView* map_view, QWidget* parent = nullptr);
      ~AssetBrowserWidget();
      std::string const& getFilename() const;;

      void set_browse_mode(asset_browse_mode browse_mode);

      asset_browse_mode _browse_mode = asset_browse_mode::world;
    signals:
        void gl_data_unloaded();
        void selectionChanged(std::string const& path);

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
      bool validateBrowseMode(const QString& wow_file_path);

      // commented objects that shouldn't be placed on the map, still accessible through Show all
      const QMap<QString, asset_browse_mode> brosweModeLabels =  {
      { "World Objects", asset_browse_mode::world },
      { "Detail Doodads", asset_browse_mode::detail_doodads },
      // { "Skybox", asset_browse_mode::skybox },
      // { "Creatures", asset_browse_mode::creatures },
      // { "Characters", asset_browse_mode::characters },
      // {"Particles", asset_browse_mode::particles },
      // {"Cameras", asset_browse_mode::cameras },
      { "Items", asset_browse_mode::items },
      { "Spells", asset_browse_mode::spells },
      { "Show All", asset_browse_mode::ALL },
      };

    protected:
      void keyPressEvent(QKeyEvent* event) override;

      void setupConnectsCommon();

    };
  }
  }
}


#endif //NOGGIT_ASSETBROWSER_HPP
