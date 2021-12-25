#ifndef NOGGIT_COMPONENT_PROJECT_LIST_ITEM_HPP
#define NOGGIT_COMPONENT_PROJECT_LIST_ITEM_HPP

#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/projectSelection/widgets/ProjectListItem.hpp>
#include <noggit/ui/windows/projectSelection/noggitredprojectpage.h>
#include "ui_noggit-red-project-page.h"

namespace Noggit::Ui::Component
{
	class ExistingProjectEnumerationComponent
	{
        friend Windows::noggitRedProjectPage;
	public:
        void BuildExistingProjectList(Noggit::Ui::Windows::noggitRedProjectPage* parent)
        {
        	parent->ui->listView->clear();

            auto applicationConfiguration = parent->_noggitApplication->GetConfiguration();
            for (const auto& dirEntry : std::filesystem::directory_iterator(applicationConfiguration->ApplicationProjectPath))
            {
                auto item = new QListWidgetItem(parent->ui->listView);
                auto projectReader = Noggit::Project::ApplicationProjectReader();
                auto project = projectReader.ReadProject(dirEntry);

                if(!project.has_value())
                    continue;

                auto projectData = Noggit::Ui::Widget::ProjectListItemData();
                projectData.ProjectVersion = project->ProjectVersion;
                projectData.ProjectDirectory = QString::fromStdString(dirEntry.path().generic_string());
                projectData.ProjectName = QString::fromStdString(project->ProjectName);
                projectData.ProjectLastEdited = QDateTime::currentDateTime().date().toString();

                auto projectListItem = new Noggit::Ui::Widget::ProjectListItem(projectData, parent->ui->listView);

                item->setData(Qt::UserRole, QVariant(projectData.ProjectName));
                item->setSizeHint(projectListItem->minimumSizeHint());
                parent->ui->listView->setItemWidget(item, projectListItem);
            }
        }
	};
}

#endif //NOGGIT_COMPONENT_PROJECT_LIST_ITEM_HPP