#ifndef NOGGIT_COMPONENT_LOAD_PROJECT_HPP
#define NOGGIT_COMPONENT_LOAD_PROJECT_HPP


#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMessageBox>
#include <QModelIndex>
#include <QTextStream>

#include <algorithm>
#include <vector>

namespace Noggit::Project
{
  class NoggitProject;
}

namespace Noggit::Ui::Windows
{
  class NoggitProjectSelectionWindow;
}

namespace Noggit::Ui::Component
{
  class LoadProjectComponent
  {
    friend Windows::NoggitProjectSelectionWindow;

  public:
    std::shared_ptr<Project::NoggitProject> loadProject(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent, QString force_project_path = "");
  };
}

#endif //NOGGIT_COMPONENT_LOAD_PROJECT_HPP
