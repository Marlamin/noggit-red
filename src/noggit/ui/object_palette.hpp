// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_OBJECT_PALETTE_HPP
#define NOGGIT_OBJECT_PALETTE_HPP

#include <noggit/ui/widget.hpp>
#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>
#include <QtWidgets/QListWidget>
#include <unordered_set>
#include <string>




class QGridLayout;
class QPushButton;
class QDropEvent;
class QDragEnterEvent;
class QMouseEvent;
class QListWidget;
class QPoint;
class MapView;


namespace Noggit
{
    namespace Ui
    {
        class current_texture;

        class ObjectList : public QListWidget
        {
        public:
            ObjectList(QWidget* parent);
            void mouseMoveEvent(QMouseEvent* event) override;
            void mousePressEvent(QMouseEvent* event) override;

        private:
            QPoint _start_pos;

        };

        class ObjectPalette : public widget
        {
        Q_OBJECT

        public:
            ObjectPalette(MapView* map_view, QWidget* parent);
            ~ObjectPalette();

            void addObject();
            void addObjectByFilename(QString const& filename);

            void removeObject(QString filename);

            void removeSelectedTexture();

            void dragEnterEvent(QDragEnterEvent* event) override;
            void dropEvent(QDropEvent* event) override;

        signals:
            void selected(std::string);

        private:

            QGridLayout* layout;

            QListWidget* _object_list;
            QPushButton* _add_button;
            QPushButton* _remove_button;
            std::unordered_set<std::string> _object_paths;
            MapView* _map_view;
            Noggit::Ui::Tools::PreviewRenderer* _preview_renderer;

        };
    }
}

#endif //NOGGIT_OBJECT_PALETTE_HPP
