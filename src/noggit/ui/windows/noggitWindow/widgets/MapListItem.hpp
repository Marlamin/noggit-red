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
        QString map_name;
        int map_id;
        int map_type_id;
        int expansion_id;
        bool pinned;
    };

    class MapListItem : public QWidget
    {
        Q_OBJECT
    private:
        QLabel* _map_icon;
        QLabel* _map_name;
        QLabel* _map_id;
        QLabel* _map_instance_type;
        QLabel* _map_pinned_label;
        int _max_width;
        MapListData _map_data;

    public:
        MapListItem(const MapListData& data, QWidget* parent);
        QSize minimumSizeHint() const override;

        const QString name() { return _map_data.map_name; };
        int id() { return _map_data.map_id; };
        int type() { return _map_data.map_type_id; };
        int expansion() { return _map_data.expansion_id; };

    private:
        QString toCamelCase(const QString& s);
    };
}

#endif //NOGGIT_WIGDET_MAP_LIST_ITEM_HPP
