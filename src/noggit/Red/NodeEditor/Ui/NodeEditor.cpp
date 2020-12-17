// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NodeEditor.hpp"
#include <noggit/ui/FramelessWindow.hpp>
#include <noggit/ui/font_awesome.hpp>

#include <QFileDialog>

using namespace noggit::Red::NodeEditor::Ui;
using namespace noggit::Red::NodeEditor::Nodes;

NodeEditorWidget::NodeEditorWidget(QWidget *parent)
: QMainWindow(parent, Qt::Window)
{
  auto body = new QWidget(this);
  ui = new ::Ui::NodeEditor;
  ui->setupUi(body);
  setCentralWidget(body);

  auto titlebar = new QWidget(this);
  ui::setupFramelessWindow(titlebar, this, minimumSize(), maximumSize(), true);
  setMenuWidget(titlebar);

  setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowStaysOnTopHint);

  _model = new QFileSystemModel(this);
  auto root_index = _model->setRootPath(QDir("./scripts/").absolutePath());
  _model->setFilter(QDir::Files | QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot);

  QStringList filters;
  filters << "*.ns";

  _model->setNameFilters(filters);
  _model->setNameFilterDisables(false);
  _model->setReadOnly(false);

  ui->scriptsTree->setModel(_model);
  ui->scriptsTree->setRootIndex(root_index);
  ui->scriptsTree->hideColumn(1);
  ui->scriptsTree->hideColumn(2);
  ui->scriptsTree->hideColumn(3);

  ::setStyle();

  ui->nodeArea->setTabsClosable(true);
  ui->nodeArea->setMovable(true);

  connect(ui->scriptsTree->selectionModel(), &QItemSelectionModel::selectionChanged
      ,[=] (const QItemSelection& selected, const QItemSelection& deselected)
          {
              for (auto index : selected.indexes())
              {
                auto path = _model->filePath(index);
                loadScene(path);
                break;
              }
          }
  );

  connect(ui->searchButton, &QPushButton::clicked
      , [this]()
      {
        QStringList filters_search;

        auto search_text = ui->filterText->text();
        if (search_text.count())
          filters_search << "*" + search_text + "*.ns";
        else
          filters_search << "*.ns";

        _model->setNameFilters(filters_search);
      });

  ui->newButton->setIcon(noggit::ui::font_awesome_icon(ui::font_awesome::icons::file));
  connect(ui->newButton, &QPushButton::clicked
      , [this]()
          {
            auto scene = new NodeScene(::registerDataModels());
            auto node_view = new FlowView(scene);

            auto tab = new QWidget();
            auto layout = new QVBoxLayout(tab);
            layout->addWidget(node_view);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);

            ui->nodeArea->addTab(tab, noggit::ui::font_awesome_icon(ui::font_awesome::icons::networkwired), "New scene *");

            connect(node_view, &FlowView::changed,
                    [=]
                    {
                        int tab_idx = ui->nodeArea->currentIndex();
                        auto text = ui->nodeArea->tabText(tab_idx);

                        if (!text.endsWith(" *"))
                        {
                          ui->nodeArea->setTabText(tab_idx, text + " *");
                        }

                      });

          });

  ui->loadButton->setIcon(noggit::ui::font_awesome_icon(ui::font_awesome::icons::folderopen));
  connect(ui->loadButton, &QPushButton::clicked
      , [this]()
          {

            auto path = QFileDialog::getOpenFileName(nullptr,
                                                     tr("Open Noggit Script file"),
                                                     "./scripts/",
                                                     tr("Noggit Script files (*.ns)"));

            loadScene(path);
          });

  ui->saveButton->setIcon(noggit::ui::font_awesome_icon(ui::font_awesome::icons::save));
  connect(ui->saveButton, &QPushButton::clicked
      , [this]()
          {
            auto tab = ui->nodeArea->currentWidget();
            if (!tab)
              return;

            auto scene = static_cast<FlowView*>(tab->layout()->itemAt(0)->widget())->getScene();
            scene->save();
            ui->nodeArea->setTabText(ui->nodeArea->currentIndex(), scene->getSceneName());

            int tab_idx = ui->nodeArea->currentIndex();
            auto text = ui->nodeArea->tabText(tab_idx);

            if (text.endsWith(" *"))
            {
              text.chop(2);
              ui->nodeArea->setTabText(tab_idx, text);
            }

          });

  ui->saveAllButton->setIcon(noggit::ui::font_awesome_icon(ui::font_awesome::icons::save));
  connect(ui->saveAllButton, &QPushButton::clicked
      , [this]()
          {
            for (int i = 0; i < ui->nodeArea->count(); ++i)
            {
              auto tab = ui->nodeArea->widget(i);

              auto scene = static_cast<FlowView*>(tab->layout()->itemAt(0)->widget())->getScene();
              scene->save();
              ui->nodeArea->setTabText(i, scene->getSceneName());

              auto text = ui->nodeArea->tabText(i);

              if (text.endsWith(" *"))
              {
                text.chop(2);
                ui->nodeArea->setTabText(i, text);
              }
            }

          });

  ui->executeButton->setIcon(noggit::ui::font_awesome_icon(ui::font_awesome::icons::play));
  connect(ui->executeButton, &QPushButton::clicked
      , [this]()
          {
            auto tab = ui->nodeArea->currentWidget();
            if (!tab)
              return;

            auto scene = static_cast<FlowView*>(tab->layout()->itemAt(0)->widget())->getScene();
            static_cast<NodeScene*>(scene)->execute();

          });
}

NodeEditorWidget::~NodeEditorWidget()
{

}

void NodeEditorWidget::loadScene(const QString& filepath)
{
  for (int i = 0; i < ui->nodeArea->count(); ++i)
  {
    auto tab = ui->nodeArea->widget(i);

    auto scene = static_cast<FlowView*>(tab->layout()->itemAt(0)->widget())->getScene();

    if (QDir("./scripts/").relativeFilePath(filepath) == scene->getRelativePath())
    {
      ui->nodeArea->setCurrentIndex(i);
      return;
    }

  }

  auto scene = new NodeScene(::registerDataModels());
  auto node_view = new FlowView(scene);

  auto tab = new QWidget();
  auto layout = new QVBoxLayout(tab);
  layout->addWidget(node_view);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  scene->load(filepath);

  ui->nodeArea->addTab(tab, noggit::ui::font_awesome_icon(ui::font_awesome::icons::networkwired), scene->getSceneName());

  connect(node_view, &FlowView::changed,
          [=]
          {
              int tab_idx = ui->nodeArea->currentIndex();
              auto text = ui->nodeArea->tabText(tab_idx);

              if (!text.endsWith(" *"))
              {
                ui->nodeArea->setTabText(tab_idx, text + " *");
              }

          });
}
