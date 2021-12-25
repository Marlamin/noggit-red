#include <noggit/ui/windows/projectCreation/NoggitProjectCreationDialog.h>
#include <ui_NoggitProjectCreationDialog.h>
#include <QFileDialog>
#include <QSettings>

NoggitProjectCreationDialog::NoggitProjectCreationDialog(ProjectInformation& projectInformation, QWidget *parent) :
    QDialog(parent),
    ui(new ::Ui::NoggitProjectCreationDialog), _projectInformation(projectInformation)
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

NoggitProjectCreationDialog::~NoggitProjectCreationDialog()
{
    delete ui;
}
