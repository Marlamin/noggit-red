// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>

#include <QtWidgets/QTableWidget>

#include <string>

namespace Noggit
{
    namespace Ui
    {
        class model_list : public widget
        {
        private:

        public:
            model_list(QWidget* parent);
            QTableWidget* _table;
            void addModel(unsigned int uid, std::size_t tileX, std::size_t tileZ, const std::string filename);
            void clearList();
            virtual QSize sizeHint() const override { return QSize(560, 500); };
        };
    }
}
