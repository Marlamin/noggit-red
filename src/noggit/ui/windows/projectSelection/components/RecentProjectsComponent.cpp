// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RecentProjectsComponent.hpp"

using namespace Noggit::Ui::Component;


void RecentProjectsComponent::buildRecentProjectsList(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent)
{
  parent->_ui->listView->clear();

  auto application_configuration = parent->_noggit_application->getConfiguration();

  QSettings settings;
  int size = settings.beginReadArray("recent_projects");

  for (int i = 0; i < size; ++i)
  {
    settings.setArrayIndex(i);
    std::filesystem::path project_path = settings.value("project_path").toString().toStdString().c_str();

    auto item = new QListWidgetItem(parent->_ui->listView);
    auto project_reader = Noggit::Project::ApplicationProjectReader();

    auto project = project_reader.ReadProject(project_path);


    if (!project.has_value())
      continue;

    auto project_data = Noggit::Ui::Widget::ProjectListItemData();
    project_data.ProjectVersion = project->projectVersion;
    project_data.ProjectDirectory = QString::fromStdString(project_path.generic_string());
    project_data.ProjectName = QString::fromStdString(project->ProjectName);
    project_data.ProjectLastEdited = QDateTime::currentDateTime().date().toString();

    auto project_list_item = new Noggit::Ui::Widget::ProjectListItem(project_data, parent->_ui->listView);

    item->setData(Qt::UserRole, QVariant(QString(project_path.c_str())));
    item->setSizeHint(project_list_item->minimumSizeHint());

    QObject::connect(project_list_item, &QListWidget::customContextMenuRequested,
    [=](const QPoint& pos)
    {
      QMenu context_menu(project_list_item->tr("Context menu"), project_list_item);

      QAction action_1("Delete Project", project_list_item);
      auto icon = QIcon();
      icon.addPixmap(FontAwesomeIcon(FontAwesome::trash).pixmap(QSize(16, 16)));
      action_1.setIcon(icon);

      QObject::connect(&action_1, &QAction::triggered, [=]()
      {
        parent->handleContextMenuProjectListItemDelete(project_data.ProjectDirectory.toStdString());
      });

      context_menu.addAction(&action_1);
      context_menu.exec(project_list_item->mapToGlobal(pos));
    });


    parent->_ui->listView->setItemWidget(item, project_list_item);

  }

  settings.endArray();
}