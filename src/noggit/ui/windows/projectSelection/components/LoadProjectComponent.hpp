#ifndef NOGGIT_COMPONENT_LOAD_PROJECT_HPP
#define NOGGIT_COMPONENT_LOAD_PROJECT_HPP

#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.hpp>

#include <algorithm>
#include <cctype>
#include <vector>
#include <fstream>
#include <algorithm>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

namespace Noggit::Ui::Component
{
  class LoadProjectComponent
  {
    friend Windows::NoggitProjectSelectionWindow;

  public:
    std::shared_ptr<Project::NoggitProject> loadProject(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent)
    {
      QModelIndex index = parent->_ui->listView->currentIndex();
      auto application_configuration = parent->_noggit_application->getConfiguration();
      auto application_projects_folder_path = std::filesystem::path(application_configuration->ApplicationProjectPath);
      QString project_path = index.data(Qt::UserRole).toString();
      auto application_project_service = Noggit::Project::ApplicationProject(application_configuration);

      if (!QDir(project_path).exists())
        return {};

      // check if current filesystem is case sensitive
      QDir q_project_path{project_path};

      bool is_case_sensitive_fs = false;
      QString file_1_path { q_project_path.filePath("__noggit_fs_test.t") };
      QString file_2_path { q_project_path.filePath("__NOGGIT_FS_TEST.t") };

      QFile file_1 {file_1_path};
      if (file_1.open(QIODevice::ReadWrite))
      {
        QTextStream stream(&file_1);
        stream << "a" << Qt::endl;
      }
      else
      {
        assert(false);
        return {};
      }
      file_1.close();

      QFile file_2 {file_2_path};
      if (file_2.open(QIODevice::ReadWrite))
      {
        QTextStream stream(&file_2);
        stream << "b" << Qt::endl;
      }
      else
      {
        assert(false);
        return {};
      }
      file_2.close();

      // read the file contents
      QFile file_test {file_1_path};
      if (file_test.open(QIODevice::ReadOnly))
      {
        QTextStream stream(&file_test);
        QString line = stream.readLine();
        if (line.contains("a"))
        {
          is_case_sensitive_fs = true;
        }
      }
      else
      {
        assert(false);
        return {};
      }
      file_test.close();
      file_1.remove();

      if (is_case_sensitive_fs)
      {
        file_2.remove();

        // scan directory for non-lowercase entries
        bool has_uppercase = false;
        QDirIterator it(project_path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

        while(it.hasNext())
        {
          QString filepath = it.next();
          QString remainder = q_project_path.relativeFilePath(filepath);

          if (!remainder.isLower())
          {
            has_uppercase = true;
            break;
          }
        }

        if (has_uppercase)
        {
          QMessageBox prompt;
          prompt.setWindowIcon(QIcon(":/icon"));
          prompt.setWindowTitle("Convert project?");
          prompt.setIcon(QMessageBox::Warning);
          prompt.setWindowFlags(Qt::WindowStaysOnTopHint);
          prompt.setText("Your project contains upper-case named files, "
                         "which won't be visible to Noggit running on your OS with case-sensitive filesystems. "
                         "Do you want to fix your filenames?");
          prompt.addButton("Accept", QMessageBox::AcceptRole);
          prompt.setDefaultButton(prompt.addButton("Cancel", QMessageBox::RejectRole));
          prompt.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

          prompt.exec();

          switch (prompt.buttonRole(prompt.clickedButton()))
          {
            case QMessageBox::AcceptRole:
            {
              std::vector<QString> incorrect_paths;

              QDirIterator it (project_path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

              while(it.hasNext())
              {
                QString filepath = it.next();
                if (!filepath.isLower())
                {
                  incorrect_paths.push_back(filepath);
                }
              }

              std::sort(incorrect_paths.begin(), incorrect_paths.end(), std::greater<QString>{});

              for (auto& path : incorrect_paths)
              {
                QFileInfo f_info_path{path};
                QString filename_lower = f_info_path.fileName().toLower();
                QDir path_dir = f_info_path.dir();
                QFile::rename(path, path_dir.filePath(filename_lower));
              }

              break;
            }
            case QMessageBox::DestructiveRole:
            default:
              return {};
          }
        }
      }

      auto project = application_project_service.loadProject(project_path.toStdString());

      //This to not be static, but its hard to remove
      if (project)
        Noggit::Application::NoggitApplication::instance()->setClientData(project->ClientData);

      return project;
    }
  };
}

#endif //NOGGIT_COMPONENT_LOAD_PROJECT_HPP