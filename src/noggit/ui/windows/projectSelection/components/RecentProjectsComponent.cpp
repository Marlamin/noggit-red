// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RecentProjectsComponent.hpp"
#include "ui_NoggitProjectSelectionWindow.h"

#include <noggit/project/ApplicationProjectReader.h>
#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.hpp>
#include <noggit/ui/windows/projectSelection/widgets/ProjectListItem.hpp>

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QMenu>
#include <QProcess>
#include <QSettings>
#include <QUrl>

#include <filesystem>

using namespace Noggit::Ui::Component;


void RecentProjectsComponent::buildRecentProjectsList(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent)
{
  parent->_ui->listView->clear();

  // auto application_configuration = parent->_noggit_application->getConfiguration();

  QSettings settings;
  settings.sync();
  int favorite_proj_idx = settings.value("favorite_project", -1).toInt();
  int size = settings.beginReadArray("recent_projects");

  for (int i = 0; i < size; ++i)
  {
    settings.setArrayIndex(i);
    std::filesystem::path project_path = settings.value("project_path").toString().toStdString().c_str();

    if (!std::filesystem::exists(project_path) || !std::filesystem::is_directory(project_path))
      continue;

    auto project_reader = Noggit::Project::ApplicationProjectReader();

    auto project = project_reader.readProject(project_path);

    if (!project.has_value())
      continue;

    auto item = new QListWidgetItem(parent->_ui->listView);

    auto project_data = Noggit::Ui::Widget::ProjectListItemData();
    project_data.project_version = project->projectVersion;
    project_data.project_directory = QString::fromStdString(project_path.generic_string());
    project_data.project_name = QString::fromStdString(project->ProjectName);
    project_data.project_last_edited = QDateTime::currentDateTime().date().toString();
    project_data.is_favorite = favorite_proj_idx == i ? true : false;

    auto project_list_item = new Noggit::Ui::Widget::ProjectListItem(project_data, parent->_ui->listView);

    item->setData(Qt::UserRole, QVariant(QString(project_path.string().c_str())));
    item->setSizeHint(project_list_item->minimumSizeHint());

    QObject::connect(project_list_item, &QListWidget::customContextMenuRequested,
    [=](const QPoint& pos)
    {
      QMenu context_menu(project_list_item->tr("Context menu"), project_list_item);

      // removing the option to delete project files due to people accidentally removing their work...
      /*
      QAction action_1("Delete Project", project_list_item);
      action_1.setIcon(FontAwesomeIcon(FontAwesome::trash).pixmap(QSize(16, 16)));

      QObject::connect(&action_1, &QAction::triggered, [=]()
      {
        parent->handleContextMenuProjectListItemDelete(project_data.project_directory.toStdString());
      });

      context_menu.addAction(&action_1);
      */

      QAction action_2("Forget Project", project_list_item);
      action_2.setIcon(FontAwesomeIcon(FontAwesome::cloud).pixmap(QSize(16, 16)));

      QObject::connect(&action_2, &QAction::triggered, [=]()
      {
        parent->handleContextMenuProjectListItemForget(project_data.project_directory.toStdString());
      });

      context_menu.addAction(&action_2);
      /////
      QAction action_3("Open Project Directory", project_list_item);
      action_3.setIcon(FontAwesomeIcon(FontAwesome::folderopen).pixmap(QSize(16, 16)));

      QObject::connect(&action_3, &QAction::triggered, [=]()
          {
              openDirectory(project_data.project_directory.toStdString());
          });

      context_menu.addAction(&action_3);
      /////
      QAction action_4("Open WoW Client Directory", project_list_item);
      action_4.setIcon(FontAwesomeIcon(FontAwesome::gamepad).pixmap(QSize(16, 16)));

      QObject::connect(&action_4, &QAction::triggered, [=]()
          {
              openDirectory(project->ClientPath);
          });

      context_menu.addAction(&action_4);

      // if (!project_data.is_favorite)
        QAction action_5("Favorite Project(auto load)", project_list_item);
        auto fav_icon = QIcon();
        fav_icon.addPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16, 16)));
        action_5.setIcon(fav_icon);

        if (project_data.is_favorite)
            action_5.setText("Unfavorite Project");

        QObject::connect(&action_5, &QAction::triggered, [=]()
            {
                if (!project_data.is_favorite)
                    parent->handleContextMenuProjectListItemFavorite(i);
                else
                    parent->handleContextMenuProjectListItemFavorite(-1);
                // QSettings settings;
                // if (!project_data.is_favorite)
                //     settings.setValue("favorite_project", i);
                // else
                //     settings.setValue("favorite_project", -1);
                // buildRecentProjectsList(parent);
            });
        context_menu.addAction(&action_5);
      // else
      // {
      //   QAction action_6("Unfavorite Project", project_list_item);
      //   auto fav_icon = QIcon();
      //   fav_icon.addPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16, 16)));
      //   action_6.setIcon(fav_icon);
      // 
      //   QObject::connect(&action_6, &QAction::triggered, [=]()
      //       {
      //           // TODO
      //           // parent->handleContextMenuProjectListItemFavorite();
      //           QSettings settings;
      //           settings.setValue("favorite_project", -1);
      //           // project_data.is_favorite = false;
      //       });
      //   context_menu.addAction(&action_6);
      // }

      /////
      context_menu.exec(project_list_item->mapToGlobal(pos));
    });


    parent->_ui->listView->setItemWidget(item, project_list_item);

  }

  settings.endArray();
}

void RecentProjectsComponent::openDirectory(std::string const& directory_path)
{
    if (!std::filesystem::exists(directory_path) || !std::filesystem::is_directory(directory_path))
        return;
    auto path = QString(directory_path.c_str());
    QFileInfo info(path);
#if defined(Q_OS_WIN)
    QStringList args;
    if (!info.isDir())
        args << "/select,";
    args << QDir::toNativeSeparators(path);
    if (QProcess::startDetached("explorer", args))
        return;
#elif defined(Q_OS_MAC)
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + path + "\"";
    args << "-e";
    args << "end tell";
    args << "-e";
    args << "return";
    if (!QProcess::execute("/usr/bin/osascript", args))
        return;
#endif
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));

}

void RecentProjectsComponent::registerProjectChange(std::string const& project_path)
{
  QSettings settings;
  settings.sync();

  QList<QString> recent_projects;

  std::size_t size = settings.beginReadArray("recent_projects");
  for (int i = 0; i < size; ++i)
  {
    settings.setArrayIndex(i);
    QString p_path = settings.value("project_path").toString();

    std::string std_project_path = p_path.toStdString();

    if (std::filesystem::exists(std_project_path)
      && std::filesystem::is_directory(std_project_path))
    {
      recent_projects.append(p_path);
    }
  }
  settings.endArray();

  // remove duplicates
  auto it = std::find(recent_projects.begin(), recent_projects.end(), project_path.c_str());

  if (it != recent_projects.end())
  {
    recent_projects.erase(it);
  }

  settings.remove("recent_projects");
  settings.beginWriteArray("recent_projects");

  settings.setArrayIndex(0);
  settings.setValue("project_path", QString(project_path.c_str()));

  int index_counter = 1;
  for (auto& p_path : recent_projects)
  {
    settings.setArrayIndex(index_counter);
    settings.setValue("project_path", p_path);
    index_counter++;
  }

  settings.endArray();

  settings.sync();

}

void RecentProjectsComponent::registerProjectRemove(std::string const& project_path)
{
  QSettings settings;
  settings.sync();

  QList<QString> recent_projects;

  std::size_t size = settings.beginReadArray("recent_projects");
  for (int i = 0; i < size; ++i)
  {
    settings.setArrayIndex(i);
    recent_projects.append(settings.value("project_path").toString());
  }
  settings.endArray();

  auto it = std::find(recent_projects.begin(), recent_projects.end(), project_path.c_str());

  if (it != recent_projects.end())
  {
    recent_projects.erase(it);
    size--;

    settings.remove("recent_projects");
    settings.beginWriteArray("recent_projects");

    for (int i = 0; i < size; ++i)
    {
      settings.setArrayIndex(i);
      settings.setValue("project_path", recent_projects[i]);
    }
    settings.endArray();

    settings.sync();
  }


}

