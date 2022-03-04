#ifndef NOGGIT_COMPONENT_EXISTING_PROJECT_ENUMERATION_HPP
#define NOGGIT_COMPONENT_EXISTING_PROJECT_ENUMERATION_HPP

#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/projectSelection/widgets/ProjectListItem.hpp>
#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.h>
#include "ui_NoggitProjectSelectionWindow.h"

namespace Noggit::Ui::Component
{
	class ExistingProjectEnumerationComponent
	{
        friend Windows::NoggitProjectSelectionWindow;
	public:
        void BuildExistingProjectList(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent)
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
                projectData.ProjectVersion = project->projectVersion;
                projectData.ProjectDirectory = QString::fromStdString(dirEntry.path().generic_string());
                projectData.ProjectName = QString::fromStdString(project->ProjectName);
                projectData.ProjectLastEdited = QDateTime::currentDateTime().date().toString();

                auto projectListItem = new Noggit::Ui::Widget::ProjectListItem(projectData, parent->ui->listView);

                item->setData(Qt::UserRole, QVariant(projectData.ProjectName));
                item->setSizeHint(projectListItem->minimumSizeHint());

                QObject::connect(projectListItem, &QListWidget::customContextMenuRequested,
                    [=](const QPoint& pos)
                    {
                        QMenu contextMenu(projectListItem->tr("Context menu"), projectListItem);

                        QAction action1("Delete Project", projectListItem);
                        auto icon = QIcon();
                        icon.addPixmap(FontAwesomeIcon(FontAwesome::trash).pixmap(QSize(16, 16)));
                        action1.setIcon(icon);

                        QObject::connect(&action1, &QAction::triggered, [=]()
                        {
                        	parent->HandleContextMenuProjectListItemDelete(projectData.ProjectDirectory.toStdString());
                        });

                        contextMenu.addAction(&action1);
                        contextMenu.exec(projectListItem->mapToGlobal(pos));
                    });


                parent->ui->listView->setItemWidget(item, projectListItem);
            }
        }
	};
}

#endif //NOGGIT_COMPONENT_EXISTING_PROJECT_ENUMERATION_HPP