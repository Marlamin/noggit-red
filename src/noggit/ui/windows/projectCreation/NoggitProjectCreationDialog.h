#ifndef NOGGIT_PROJECT_CREATION_DIALOG_HPP
#define NOGGIT_PROJECT_CREATION_DIALOG_HPP

#include <QDialog>

namespace Ui {
class NoggitProjectCreationDialog;
}


struct ProjectInformation
{
    std::string project_name;
    std::string project_path;
    std::string game_client_path;
    std::string game_client_version;
};

class NoggitProjectCreationDialog : public QDialog
{
    Q_OBJECT

public:
    NoggitProjectCreationDialog(ProjectInformation& project_information, QWidget *parent = nullptr);
    ~NoggitProjectCreationDialog();

private:
    ::Ui::NoggitProjectCreationDialog*ui;
    ProjectInformation& _project_information;
};

#endif //NOGGIT_PROJECT_CREATION_DIALOG_HPP
