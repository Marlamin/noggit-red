#ifndef NOGGIT_PROJECT_CREATION_DIALOG_HPP
#define NOGGIT_PROJECT_CREATION_DIALOG_HPP

#include <QDialog>

namespace Ui {
class NoggitProjectCreationDialog;
}


struct ProjectInformation
{
    std::string ProjectName;
    std::string GameClientPath;
    std::string GameClientVersion;
};

class NoggitProjectCreationDialog : public QDialog
{
    Q_OBJECT

public:
    NoggitProjectCreationDialog(ProjectInformation& projectInformation, QWidget *parent = nullptr);
    ~NoggitProjectCreationDialog();

private:
    ::Ui::NoggitProjectCreationDialog*ui;
    ProjectInformation& _projectInformation;
};

#endif //NOGGIT_PROJECT_CREATION_DIALOG_HPP
