#ifndef NOGGIT_WIGDET_MAP_LIST_ITEM_HPP
#define NOGGIT_WIGDET_MAP_LIST_ITEM_HPP

#include <QMenuBar>
#include <QAction>
#include <qgraphicseffect.h>
#include <QGridLayout>
#include <QString>
#include <QWidget>
#include <noggit/project/ApplicationProject.h>

namespace Noggit::Ui::Widget
{
    struct MapListData
    {
        QString MapName;
        int MapId;
        int MapTypeId;
        int ExpansionId;
    };

    class MapListItem : public QWidget
    {
        Q_OBJECT
    private:
        QLabel* project_version_icon;
        QLabel* project_name_label;
        QLabel* project_directory_label;
        QLabel* project_version_label;
        QLabel* project_last_edited_label;
        int _maxWidth;
    public:
        MapListItem(const MapListData& data, QWidget* parent);
        QSize minimumSizeHint() const override;
    private:
        QString toCamelCase(const QString& s);
    };
}

#endif //NOGGIT_WIGDET_MAP_LIST_ITEM_HPP