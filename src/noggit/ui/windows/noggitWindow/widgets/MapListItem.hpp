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
        bool Pinned;
    };

    class MapListItem : public QWidget
    {
        Q_OBJECT
    private:
        QLabel* map_icon;
        QLabel* map_name;
        QLabel* map_id;
        QLabel* map_instance_type;
        QLabel* map_pinned_label;
        int _maxWidth;
    public:
        MapListItem(const MapListData& data, QWidget* parent);
        QSize minimumSizeHint() const override;
    private:
        QString toCamelCase(const QString& s);
    };
}

#endif //NOGGIT_WIGDET_MAP_LIST_ITEM_HPP