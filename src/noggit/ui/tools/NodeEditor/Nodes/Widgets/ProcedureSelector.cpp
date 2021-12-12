// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ProcedureSelector.hpp"
#include <noggit/ui/font_awesome.hpp>

#include <QDir>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidgetAction>
#include <QLineEdit>
#include <QTreeView>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QMenu>
#include <QHeaderView>


using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ProcedureSelector::ProcedureSelector(QWidget *parent)
{
  setAttribute(Qt::WA_TranslucentBackground);

  auto layout = new QHBoxLayout(this);

  auto search_button = new QPushButton("None", this);
  layout->addWidget(search_button);
  search_button->setIcon(Noggit::Ui::font_awesome_icon(Noggit::Ui::font_awesome::icons::networkwired));
  search_button->setToolTip("Select procedure");
  search_button->adjustSize();

  auto menu = new QMenu(search_button);
  menu->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  menu->setMinimumSize(200, 300);
  menu->setMaximumSize(400, 600);
  search_button->setMenu(menu);

  auto action = new QWidgetAction(menu);
  menu->addAction(action);

  auto action_widget = new QWidget(menu);
  action->setDefaultWidget(action_widget);

  auto search_text = new QLineEdit(menu);

  auto action_layout = new QVBoxLayout(menu);
  action_layout->setContentsMargins(1, 1, 1, 1);
  action_layout->setSpacing(0);

  action_layout->addWidget(search_text);

  auto tree = new QTreeView(menu);
  tree->setMouseTracking(true);

  action_layout->addWidget(tree);

  auto model = new QFileSystemModel(menu);

  auto root_index = model->setRootPath(QDir("./scripts/").absolutePath());
  model->setFilter(QDir::Files | QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot);

  QStringList filters;
  filters << "*.ns";

  model->setNameFilters(filters);
  model->setNameFilterDisables(false);

  tree->setModel(model);
  tree->setRootIndex(root_index);
  tree->hideColumn(1);
  tree->hideColumn(2);
  tree->hideColumn(3);
  tree->header()->close();

  QMenu::connect(menu, &QMenu::aboutToShow, [=] { search_text->setFocus(); });

  QTreeView::connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged
      ,[=] (const QItemSelection& selected, const QItemSelection& deselected)
                     {
                         for (auto index : selected.indexes())
                         {
                           auto path = model->filePath(index);
                           auto text = QDir("./scripts/").relativeFilePath(path);
                           search_button->setText(text);
                           search_button->adjustSize();

                           emit entry_updated(text);

                           menu->close();
                           break;
                         }
                     }
  );

  QLineEdit::connect(search_text, &QLineEdit::textChanged
      ,[=]
                     {
                         QStringList filters_search;

                         auto text = search_text->text();
                         if (text.count())
                           filters_search << "*" + text + "*.ns";
                         else
                           filters_search << "*.ns";

                         model->setNameFilters(filters_search);
                     });
}

QString ProcedureSelector::getPath()
{
  return static_cast<QPushButton*>(layout()->itemAt(0)->widget())->text();
}

void ProcedureSelector::setPath(QString const& path)
{
  static_cast<QPushButton*>(layout()->itemAt(0)->widget())->setText(path);
}

