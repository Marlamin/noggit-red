// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/ModelList.h>

#include <QVBoxLayout>
#include <QScrollArea>

namespace Noggit
{
    namespace Ui
    {
        model_list::model_list(QWidget* parent)
            : widget(parent, Qt::Window)
        {
            setWindowFlags(Qt::Tool);
            setMinimumWidth(560);

            auto layout(new QVBoxLayout(this));

            auto scroll_area = new QScrollArea(this);
            scroll_area->setWidget(_table = new QTableWidget(this));
            scroll_area->setWidgetResizable(true);

            _table->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			_table->setColumnCount(4);
            _table->setColumnWidth(0, 10);
            _table->setColumnWidth(1, 20);
			_table->setColumnWidth(2, 20);
			_table->setColumnWidth(3, 500);
			_table->setHorizontalHeaderLabels({ "UID", "TileX", "TileY", "Filename" });

            // Make read only
            _table->setEditTriggers(QAbstractItemView::NoEditTriggers);

            layout->addWidget(scroll_area);
        }

        void model_list::addModel(unsigned int uid, std::size_t tileX, std::size_t tileZ, const std::string filename)
        {
			auto row = _table->rowCount();
			_table->insertRow(row);

			auto uidItem = new QTableWidgetItem(QString::number(uid));
			_table->setItem(row, 0, uidItem);

			auto tileXItem = new QTableWidgetItem(QString::number(tileX));
			_table->setItem(row, 1, tileXItem);

			auto tileZItem = new QTableWidgetItem(QString::number(tileZ));
			_table->setItem(row, 2, tileZItem);

			auto filenameItem = new QTableWidgetItem(QString::fromStdString(filename));
            _table->setItem(row, 3, filenameItem);
        }

        void model_list::clearList() {
			_table->clearContents();
			_table->setRowCount(0);
        }
    }
}
