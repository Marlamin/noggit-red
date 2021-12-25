#ifndef NOGGIT_WIGDET_PROJECT_LIST_ITEM_HPP
#define NOGGIT_WIGDET_PROJECT_LIST_ITEM_HPP

#include <QMenuBar>
#include <QAction>
#include <qgraphicseffect.h>
#include <QGridLayout>
#include <QString>
#include <QWidget>
#include <noggit/project/ApplicationProject.h>

namespace Noggit::Ui::Widget
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

#endif //NOGGIT_WIGDET_PROJECT_LIST_ITEM_HPP