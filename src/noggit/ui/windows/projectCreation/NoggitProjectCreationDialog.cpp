#include <noggit/ui/windows/projectCreation/NoggitProjectCreationDialog.h>
#include <ui_NoggitProjectCreationDialog.h>
#include <QFileDialog>
#include <QSettings>

NoggitProjectCreationDialog::NoggitProjectCreationDialog(ProjectInformation& project_information, QWidget* parent) :
    QDialog(parent), ui(new ::Ui::NoggitProjectCreationDialog), _projectInformation(project_information)
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

                     QString folder_name = QFileDialog::getExistingDirectory(this
                                                                             , "Select Client Directory"
                                                                             , default_path
                                                                             , QFileDialog::ShowDirsOnly |
                                                                               QFileDialog::DontResolveSymlinks);
                     ui->clientPathField->setText(folder_name);
                   }
  );

  QObject::connect(ui->projectPathField_browse, &QPushButton::clicked, [this]
                   {
                     QString folder_name = QFileDialog::getExistingDirectory(this
                         , "Select Client Directory"
                         , "/"
                         , QFileDialog::ShowDirsOnly |
                           QFileDialog::DontResolveSymlinks);
                     ui->projectPathField->setText(folder_name);
                   }
  );

  QObject::connect(ui->button_ok, &QPushButton::clicked, [&]
                   {
                     project_information.ProjectName = ui->projectName->text().toStdString();
                     project_information.GameClientPath = ui->clientPathField->text().toStdString();
                     project_information.GameClientVersion = ui->project_expansion->currentText().toStdString();
                     project_information.ProjectPath = ui->projectPathField->text().toStdString();

                     close();
                   }
  );

  QObject::connect(ui->button_cancel, &QPushButton::clicked, [&]
                   {
                     close();
                   }
  );
}

NoggitProjectCreationDialog::~NoggitProjectCreationDialog()
{
  delete ui;
}
