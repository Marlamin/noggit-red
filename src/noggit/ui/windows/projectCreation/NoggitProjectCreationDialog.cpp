#include <noggit/ui/windows/projectCreation/NoggitProjectCreationDialog.h>
#include <ui_NoggitProjectCreationDialog.h>
#include <QFileDialog>
#include <QSettings>

NoggitProjectCreationDialog::NoggitProjectCreationDialog(ProjectInformation& projectInformation, QWidget *parent) :
    QDialog(parent)
	, ui(new ::Ui::NoggitProjectCreationDialog)
	, _projectInformation(projectInformation)
{
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    ui->setupUi(this);

    QIcon icon = QIcon(":/icon-wrath");
    ui->expansion_icon->setPixmap(icon.pixmap(QSize(32, 32)));
	ui->expansion_icon->setObjectName("icon");
    ui->expansion_icon->setStyleSheet("QLabel#icon { padding: 0px }");
    ui->projectPathField_browse->setObjectName("icon");

    QObject::connect(ui->project_expansion, QOverload<int>::of(&QComboBox::currentIndexChanged)
        , [&](int index)
        {
           auto versionSelected =  ui->project_expansion->currentText().toStdString();

           QIcon icon;
           if (versionSelected == "Wrath Of The Lich King")
               icon = QIcon(":/icon-wrath");
           if (versionSelected == "Shadowlands")
               icon = QIcon(":/icon-shadow");

           ui->expansion_icon->setPixmap(icon.pixmap(QSize(32, 32)));
        }
    );

    QObject::connect(ui->projectPathField_browse, &QPushButton::clicked
        , [&]
        {
            QSettings settings;
            auto defaultPath = settings.value("project/game_path").toString();
            ui->projectPathField->setText(defaultPath);
    
            QString folderName = QFileDialog::getExistingDirectory(parent, "Select Client Directory", defaultPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            ui->projectPathField->setText(folderName);
        }
    );
    
    QObject::connect(ui->button_ok, &QPushButton::clicked
        , [&]
        {
            projectInformation.ProjectName = ui->projectName->text().toStdString();
            projectInformation.GameClientPath = ui->projectPathField->text().toStdString();
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
