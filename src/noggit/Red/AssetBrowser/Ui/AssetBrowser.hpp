#ifndef NOGGIT_ASSETBROWSER_HPP
#define NOGGIT_ASSETBROWSER_HPP

#include <QWidget>
#include <ui_AssetBrowser.h>

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

    };
  }
}


#endif //NOGGIT_ASSETBROWSER_HPP
