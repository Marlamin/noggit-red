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
        QString project_name;
        QString project_directory;
        QString project_last_edited;
        Project::ProjectVersion project_version;
    };

    class ProjectListItem : public QWidget
    {
        Q_OBJECT
    private:
        QLabel* _project_version_icon;
        QLabel* _project_name_label;
        QLabel* _project_directory_label;
        QLabel* _project_version_label;
        QLabel* _project_last_edited_label;
    public:
        ProjectListItem(const ProjectListItemData& data, QWidget* parent);
        QSize minimumSizeHint() const override;
    private:
        QString toCamelCase(const QString& s);
    };
}

#endif //NOGGIT_WIGDET_PROJECT_LIST_ITEM_HPP