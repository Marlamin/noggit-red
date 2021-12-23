#include "projectcreationdialog.h"
#include <ui_projectcreationdialog.h>
#include <QFileDialog>
#include <QSettings>

ProjectCreationDialog::ProjectCreationDialog(ProjectInformation& projectInformation, QWidget *parent) :
    QDialog(parent),
    ui(new ::Ui::ProjectCreationDialog), _projectInformation(projectInformation)
{
    ui->setupUi(this);

    QObject::connect(ui->button_folder_select, &QPushButton::clicked
        , [&]
        {
            QSettings settings;
            auto defaultPath = settings.value("project/game_path").toString();
            ui->game_client_apth->setText(defaultPath);

            QString folderName = QFileDialog::getExistingDirectory(this, "Select Client Directory", defaultPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            ui->game_client_apth->setText(folderName);
        }
    );

    QObject::connect(ui->button_ok, &QPushButton::clicked
        , [&]
        {
            projectInformation.ProjectName = ui->project_name->text().toStdString();
            projectInformation.GameClientPath = ui->game_client_apth->text().toStdString();
            projectInformation.GameClientVersion = ui->project_expansion->currentText().toStdString();

            close();
        }
    );

    QObject::connect(ui->button_cancel, &QPushButton::clicked
        , [&]
        {
            close();
        }
    );
}

ProjectCreationDialog::~ProjectCreationDialog()
{
    delete ui;
}
