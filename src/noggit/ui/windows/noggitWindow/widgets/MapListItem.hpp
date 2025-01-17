#ifndef NOGGIT_WIGDET_MAP_LIST_ITEM_HPP
#define NOGGIT_WIGDET_MAP_LIST_ITEM_HPP

#include <QSize>
#include <QString>
#include <QWidget>

class QLabel;

namespace Noggit::Ui::Widget
{
    struct MapListData
    {
        QString map_name;
        int map_id;
        int map_type_id;
        int expansion_id;
        bool pinned;
        bool wmo_map;
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

        const QString name() const;;
        int id() const;;
        int type() const;;
        int expansion() const;;
        bool wmo_map() const;;

    private:
        QString toCamelCase(const QString& s);
    };
}

#endif //NOGGIT_WIGDET_MAP_LIST_ITEM_HPP
