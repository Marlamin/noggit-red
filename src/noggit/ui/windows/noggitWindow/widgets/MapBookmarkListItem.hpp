#ifndef NOGGIT_WIGDET_MAP_BOOKMARK_LIST_ITEM_HPP
#define NOGGIT_WIGDET_MAP_BOOKMARK_LIST_ITEM_HPP

#include <glm/vec3.hpp>

#include <QSize>
#include <QString>
#include <QWidget>

class QLabel;

namespace Noggit::Ui::Widget
{
    struct MapListBookmarkData
    {
        QString MapName;
        glm::vec3 Position;
    };

    class MapListBookmarkItem : public QWidget
    {
        Q_OBJECT
    private:
        QLabel* map_icon;
        QLabel* map_name;
        QLabel* map_position;
        int _maxWidth;
    public:
        MapListBookmarkItem(const MapListBookmarkData& data, QWidget* parent);
        QSize minimumSizeHint() const override;
    private:
        QString toCamelCase(const QString& s);
    };
}

#endif //NOGGIT_WIGDET_MAP_BOOKMARK_LIST_ITEM_HPP
