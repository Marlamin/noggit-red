// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/MapView.h>
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/WMOInstance.h>

#include <ClientData.hpp>

#include <fstream>
#include <regex>
#include <string>

#include <QSettings>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>

namespace Noggit
{
  namespace Ui
  {
    model_import::model_import (Noggit::Ui::object_editor* object_editor)
      : QWidget (object_editor, Qt::Tool | Qt::WindowStaysOnTopHint)
    {
      setWindowIcon (QIcon (":/icon"));
      auto layout (new QVBoxLayout (this));

      auto layout_filter = new QHBoxLayout(this);
      layout_filter->addWidget(new QLabel("Filter:", this));
      layout_filter->addWidget (_textBox = new QLineEdit (this));

      layout->addLayout(layout_filter);

      connect ( _textBox, &QLineEdit::textChanged
              , [this]
                {
                  buildModelList();
                }
              );

      _list = new QListWidget (this);
      layout->addWidget (_list);

      buildModelList();

      connect ( _list, &QListWidget::itemDoubleClicked
              , [object_editor] (QListWidgetItem* item)
                {
                  object_editor->copy(item->text().toStdString());
                }
              );
    }

    void model_import::buildModelList()
    {
      _list->clear();

      QSettings settings;

      std::ifstream fileReader (settings.value ("project/import_file", "import.txt").toString().toStdString());
      std::string const filter
        (BlizzardArchive::ClientData::normalizeFilenameInternal(_textBox->text().toStdString()));

      std::string line;
      while (std::getline (fileReader, line))
      {
        if (line.empty())
        {
          continue;
        }

        std::string path (BlizzardArchive::ClientData::normalizeFilenameInternal(line));

        if (!filter.empty() && path.find (filter) == std::string::npos)
        {
          continue;
        }

        std::regex regex("[^\\.]+\\.(m2|wmo)"), wmo_group(".*_[0-9]{3}\\.wmo");
        std::smatch match;

        if (std::regex_search(path, match, regex) && !regex_match(path, wmo_group))
        {
          _list->addItem (QString::fromStdString (match.str(0)));
        }
      }

      _list->setMinimumWidth(_list->sizeHintForColumn(0));
      // margin for the "Filter: " text
      setMinimumWidth(_list->sizeHintForColumn(0) + 100);
    }
  }
}
