#ifndef NOGGIT_COMPONENT_PROJECT_LIST_ITEM_HPP
#define NOGGIT_COMPONENT_PROJECT_LIST_ITEM_HPP

#include <QMenuBar>
#include <QAction>
#include <qgraphicseffect.h>
#include <QString>
#include <QWidget>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/ui/windows/mainWindow/main_window.hpp>

namespace Noggit::Ui::Component
{
    struct ProjectListItemData
    {
        QString ProjectName;
        QString ProjectDirectory;
        QString ProjectLastEdited;
        Project::ProjectVersion ProjectVersion;
    };

    class ProjectListItem : public QWidget
    {
        Q_OBJECT
    private:
        QLabel* project_version_icon;
        QLabel* project_name_label;
        QLabel* project_directory_label;
        QLabel* project_version_label;
        QLabel* project_last_edited_label;
    public:
        ProjectListItem(const ProjectListItemData& data, QWidget* parent);
        QSize minimumSizeHint() const override;
    private:
        QString toCamelCase(const QString& s);
    };
}

#endif //NOGGIT_COMPONENT_PROJECT_LIST_ITEM_HPP