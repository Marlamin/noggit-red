#ifndef PROJECTCREATIONDIALOG_H
#define PROJECTCREATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectCreationDialog;
}


struct ProjectInformation
{
    std::string ProjectName;
    std::string GameClientPath;
    std::string GameClientVersion;
};

class ProjectCreationDialog : public QDialog
{
    Q_OBJECT

public:
    ProjectCreationDialog(ProjectInformation& projectInformation, QWidget *parent = nullptr);
    ~ProjectCreationDialog();

private:
    ::Ui::ProjectCreationDialog *ui;
    ProjectInformation& _projectInformation;
};

#endif // PROJECTCREATIONDIALOG_H
