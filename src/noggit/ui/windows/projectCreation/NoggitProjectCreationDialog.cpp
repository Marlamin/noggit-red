#include <noggit/ui/windows/projectCreation/NoggitProjectCreationDialog.h>
#include <ui_NoggitProjectCreationDialog.h>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include <filesystem>

NoggitProjectCreationDialog::NoggitProjectCreationDialog(ProjectInformation& project_information, QWidget* parent)
    : QDialog(parent)
    , ui(new ::Ui::NoggitProjectCreationDialog)
    , _project_information(project_information)
{
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

  ui->setupUi(this);

  QIcon icon = QIcon(":/icon-wrath");
  ui->expansion_icon->setPixmap(icon.pixmap(QSize(32, 32)));
  ui->expansion_icon->setObjectName("icon");
  ui->expansion_icon->setStyleSheet("QLabel#icon { padding: 0px }");
  ui->clientPathField_browse->setObjectName("icon");

  QObject::connect(ui->project_expansion, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index)
                   {
                     auto version_selected = ui->project_expansion->currentText().toStdString();

                     QIcon icon;
                     if (version_selected == "Wrath Of The Lich King")
                       icon = QIcon(":/icon-wrath");
                     if (version_selected == "Shadowlands");

                     ui->expansion_icon->setPixmap(icon.pixmap(QSize(32, 32)));
                   }
  );

  QObject::connect(ui->clientPathField_browse, &QPushButton::clicked, [this]
                   {
                     // TODO: implement automatic client path detection
                     QSettings settings;
                     auto default_path = settings.value("project/game_path").toString();
                     ui->clientPathField->setText(default_path);

                     QString folder_name = QFileDialog::getExistingDirectory(this, "Select Client Directory", default_path,
                                                                             QFileDialog::ShowDirsOnly |
                                                                             QFileDialog::DontResolveSymlinks);
                     ui->clientPathField->setText(folder_name);
                   }
  );

  QObject::connect(ui->projectPathField_browse, &QPushButton::clicked, [this]
                   {
                     QString folder_name = QFileDialog::getExistingDirectory(this, "Select Project Directory", "/",
                                                                             QFileDialog::ShowDirsOnly |
                                                                             QFileDialog::DontResolveSymlinks);
                     ui->projectPathField->setText(folder_name);
                   }
  );

  QObject::connect(ui->button_ok, &QPushButton::clicked, [&]
                   {
                     project_information.project_name = ui->projectName->text().toStdString();

                     if (project_information.project_name.empty())
                     {
                       QMessageBox::critical(this, "Error", "Project must have a name.");
                       return;
                     }

                     project_information.game_client_path = ui->clientPathField->text().toStdString();

                     if (project_information.game_client_path.empty())
                     {
                       QMessageBox::critical(this, "Error", "Game client path is empty.");
                       return;
                     }

                     std::filesystem::path game_path(project_information.game_client_path);
                     if (!std::filesystem::exists(game_path))
                     {
                       QMessageBox::critical(this, "Error", "Game client path does not exist.");
                       return;
                     }

                     project_information.project_path = ui->projectPathField->text().toStdString();

                     std::filesystem::path project_path(project_information.project_path);

                     if (project_path.empty())
                     {
                       QMessageBox::critical(this, "Error", "Project path is empty.");
                       return;
                     }

                     project_information.game_client_version = ui->project_expansion->currentText().toStdString();


                     done(QDialog::Accepted);
                     close();
                   }
  );

  QObject::connect(ui->button_cancel, &QPushButton::clicked, [&]
                   {
                     done(QDialog::Rejected);
                     close();
                   }
  );
}

NoggitProjectCreationDialog::~NoggitProjectCreationDialog()
{
  delete ui;
}
