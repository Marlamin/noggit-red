#ifndef NOGGIT_ASSETBROWSER_HPP
#define NOGGIT_ASSETBROWSER_HPP

#include <ui_AssetBrowser.h>
#include <noggit/Red/AssetBrowser/Ui/Model/TreeManager.hpp>

#include <QWidget>
#include <QStandardItemModel>


namespace noggit
{
  namespace Red::AssetBrowser::Ui
  {
    class AssetBrowserWidget : public QWidget
    {
    public:
      AssetBrowserWidget(QWidget* parent = nullptr);
      ~AssetBrowserWidget();

    private:
      ::Ui::AssetBrowser* ui;
      QStandardItemModel* _model;

      void updateModelData();
      void recurseDirectory(Model::TreeManager& tree_mgr, const QString& s_dir, const QString& project_dir);

    };
  }
}


#endif //NOGGIT_ASSETBROWSER_HPP
