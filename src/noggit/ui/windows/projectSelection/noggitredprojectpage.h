#ifndef NOGGITREDPROJECTPAGE_H
#define NOGGITREDPROJECTPAGE_H

#include <QMainWindow>
#include <noggit/ui/windows/settingsPanel/SettingsPanel.h>
#include <QMenuBar>
#include <QAction>
#include <qgraphicseffect.h>
#include <QString>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/ui/windows/mainWindow/main_window.hpp>
#include <noggit/ui/windows/projectCreation/projectcreationdialog.h>
#include <noggit/ui/windows/projectSelection/components/_projectSelectionComponent.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class noggitRedProjectPage; }
QT_END_NAMESPACE

namespace Noggit::Application {
    class NoggitApplication;
}


namespace Noggit::Ui::Windows
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
        ProjectListItem(const ProjectListItemData& data, QWidget* parent = nullptr) : QWidget(parent)
        {
            auto layout = QGridLayout();

            QIcon icon;
            if(data.ProjectVersion == Project::ProjectVersion::WOTLK)
                icon = QIcon(":/icon-wrath");
            if (data.ProjectVersion == Project::ProjectVersion::SL)
                icon = QIcon(":/icon-shadow");
            project_version_icon = new QLabel("", parent);
            project_version_icon->setPixmap(icon.pixmap(QSize(48, 48)));
            project_version_icon->setGeometry(0, 5, 64, 48);


            auto projectName = toCamelCase(QString(data.ProjectName));
            project_name_label = new QLabel(projectName, parent);
            project_name_label->setGeometry(45, 5, 125, 20);
            project_name_label->setObjectName("project-title-label");
            project_name_label->setStyleSheet("QLabel#project-title-label { font-size: 15px; }");

            project_directory_label = new QLabel(data.ProjectDirectory, parent);
            project_directory_label->setGeometry(48, 20, 125, 20);
            project_directory_label->setObjectName("project-information");
            project_directory_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");

            auto directoryEffect = new QGraphicsOpacityEffect(this);
            directoryEffect->setOpacity(0.5);

            project_directory_label->setGraphicsEffect(directoryEffect);
            project_directory_label->setAutoFillBackground(true);

            QString version;
            if (data.ProjectVersion == Project::ProjectVersion::WOTLK)
                version = "Wrath Of The Lich King";
            if (data.ProjectVersion == Project::ProjectVersion::SL)
                version = "Shadowlands";

        	project_version_label = new QLabel(version, parent);
            project_version_label->setGeometry(48, 35, 125, 20);
            project_version_label->setObjectName("project-information");
            project_version_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");

            auto versionEffect = new QGraphicsOpacityEffect(this);
            versionEffect->setOpacity(0.5);

            project_version_label->setGraphicsEffect(versionEffect);
            project_version_label->setAutoFillBackground(true);

            auto width = parent->sizeHint().width();
            project_last_edited_label = new QLabel(data.ProjectLastEdited, parent);
            project_last_edited_label->setGeometry(width, 35, 125, 20);
            project_last_edited_label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            project_last_edited_label->setObjectName("project-information");
            project_last_edited_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");

            auto lastEditedEffect = new QGraphicsOpacityEffect(this);
            lastEditedEffect->setOpacity(0.5);

            project_last_edited_label->setGraphicsEffect(lastEditedEffect);
            project_last_edited_label->setAutoFillBackground(true);

            layout.addWidget(project_version_icon);
            layout.addWidget(project_name_label);
            layout.addWidget(project_directory_label);
            layout.addWidget(project_version_label);
            layout.addWidget(project_last_edited_label);
            setLayout(layout.layout());
        }

        QSize minimumSizeHint() const override
        {
            return QSize(125, 55);
        }

        QString toCamelCase(const QString& s)
        {
            QStringList parts = s.split(' ', QString::SkipEmptyParts);
            for (int i = 0; i < parts.size(); ++i)
                parts[i].replace(0, 1, parts[i][0].toUpper());

            return parts.join(" ");
        }
    };

    class noggitRedProjectPage : public QMainWindow
    {
        Q_OBJECT

    public:
        noggitRedProjectPage(Noggit::Application::NoggitApplication* noggitApplication, QWidget* parent = nullptr);
        ~noggitRedProjectPage();

    private:
        Noggit::Application::NoggitApplication* _noggitApplication;
        ::Ui::noggitRedProjectPage* ui;

        std::unique_ptr<Ui::Component::ExistingProjectEnumerationComponent> _existingProjectEnumerationComponent;
        std::shared_ptr<Noggit::Project::NoggitProject> _selectedProject;

        Noggit::Ui::settings* _settings;
        std::unique_ptr<Noggit::Ui::main_window> projectSelectionPage;


        void BuildExistingProjectList();
    };
}
#endif // NOGGITREDPROJECTPAGE_H