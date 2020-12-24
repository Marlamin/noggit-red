// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NodeEditor.hpp"
#include <noggit/ui/FramelessWindow.hpp>
#include <noggit/ui/font_awesome.hpp>
#include <noggit/Log.h>

#include <QFileDialog>
#include <QTabWidget>
#include <QMessageBox>
#include <QTime>
#include <QElapsedTimer>
#include <QModelIndex>

using namespace noggit::Red::NodeEditor::Ui;
using namespace noggit::Red::NodeEditor::Nodes;

using QtNodes::FlowView;

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

  connect(ui->scriptsTree, &QTreeView::clicked
      ,[=] (const QModelIndex& index)
        {
          auto path = _model->filePath(index);
          loadScene(path);
        }
  );

  connect(_model, &QFileSystemModel::fileRenamed
  , [this] (const QString& path, const QString& old_name, const QString& new_name)
  {
    auto relative_path_old = QDir("./scripts/").relativeFilePath(QDir(path).filePath(old_name));
    auto relative_path_new = QDir("./scripts/").relativeFilePath(QDir(path).filePath(new_name));

    for (int i = 0; i < ui->nodeArea->count(); ++i)
    {
      auto tab = ui->nodeArea->widget(i);

      auto scene = static_cast<FlowView*>(tab->layout()->itemAt(0)->widget())->getScene();

      if (scene->getRelativePath() == relative_path_old)
      {
        scene->setRelativePath(relative_path_new);
        scene->setSceneName(new_name);
        ui->nodeArea->setTabText(i, scene->getChanged() ? new_name + " *" : new_name);
      }
    }

  });

  connect(ui->nodeArea, &QTabWidget::tabCloseRequested
    , [this](int index)
    {
        QWidget* tab_item = ui->nodeArea->widget(index);

        auto scene = static_cast<FlowView*>(tab_item->layout()->itemAt(0)->widget())->getScene();

        if (scene->getChanged())
        {
          QMessageBox prompt;
          prompt.setIcon (QMessageBox::Warning);
          prompt.setWindowFlags(Qt::WindowStaysOnTopHint);
          prompt.setText ("Exit");
          prompt.setInformativeText ("This page has unsaved changes. Exit without saving?");
          prompt.setDefaultButton ( prompt.addButton ("Save", QMessageBox::AcceptRole));
          prompt.addButton ("Exit", QMessageBox::RejectRole);
          prompt.setWindowFlags (Qt::CustomizeWindowHint | Qt::WindowTitleHint);

          prompt.exec();

          switch (prompt.buttonRole(prompt.clickedButton()))
          {
            case QMessageBox::AcceptRole:
              scene->save();
              break;
            case QMessageBox::RejectRole:
              break;
            default:
              break;
          }
        }

        auto flow_view = static_cast<FlowView*>(tab_item->layout()->itemAt(0)->widget());
        flow_view->deleteScene();
        ui->nodeArea->removeTab(index);

        delete flow_view;
        delete tab_item;
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
            auto tab = new QWidget();


            ui->nodeArea->setCurrentIndex(
                ui->nodeArea->addTab(tab, noggit::ui::font_awesome_icon(ui::font_awesome::icons::networkwired)
                                          , "New scene *"));

            auto node_view = new FlowView(scene, tab);
            auto layout = new QVBoxLayout(tab);
            layout->addWidget(node_view);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);


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

  ui->clearButton->setIcon(noggit::ui::font_awesome_icon(ui::font_awesome::icons::eraser));
  connect(ui->clearButton, &QPushButton::clicked
      , [this]()
          {
              auto tab = ui->nodeArea->currentWidget();
              if (!tab)
                return;

              auto scene = static_cast<FlowView*>(tab->layout()->itemAt(0)->widget())->getScene();
              static_cast<NodeScene*>(scene)->clearScene();
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

            QElapsedTimer time = QElapsedTimer();
            time.start();

            static_cast<NodeScene*>(scene)->execute();

            LogDebug << "Time elapsed: " << QTime::fromMSecsSinceStartOfDay(time.elapsed()).toString(Qt::ISODateWithMs).toStdString() << std::endl;

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

  auto tab = new QWidget();

  scene->load(filepath);
  ui->nodeArea->setCurrentIndex(
      ui->nodeArea->addTab(tab,
                           noggit::ui::font_awesome_icon(ui::font_awesome::icons::networkwired), scene->getSceneName()));

  auto node_view = new FlowView(scene, tab);
  auto layout = new QVBoxLayout(tab);
  layout->addWidget(node_view);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

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
