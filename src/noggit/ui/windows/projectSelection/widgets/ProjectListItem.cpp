#include <noggit/ui/windows/projectSelection/widgets/ProjectListItem.hpp>

namespace Noggit::Ui::Widget
{
    ProjectListItem::ProjectListItem(const ProjectListItemData& data, QWidget* parent = nullptr) : QWidget(parent)
    {
        auto layout = QGridLayout();

        QIcon icon;
        if (data.ProjectVersion == Project::ProjectVersion::WOTLK)
            icon = QIcon(":/icon-wrath");
        if (data.ProjectVersion == Project::ProjectVersion::SL)
            icon = QIcon(":/icon-shadow");
        project_version_icon = new QLabel("", parent);
        project_version_icon->setPixmap(icon.pixmap(QSize(48, 48)));
        project_version_icon->setGeometry(0, 5, 64, 48);

        auto maxWidth = parent->sizeHint().width();

        auto projectName = toCamelCase(QString(data.ProjectName));
        project_name_label = new QLabel(projectName, parent);
        project_name_label->setGeometry(45, 5, maxWidth, 20);
        project_name_label->setObjectName("project-title-label");
        project_name_label->setStyleSheet("QLabel#project-title-label { font-size: 15px; }");

        project_directory_label = new QLabel(data.ProjectDirectory, parent);
        project_directory_label->setGeometry(48, 20, maxWidth, 20);
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
        project_version_label->setGeometry(48, 35, maxWidth, 20);
        project_version_label->setObjectName("project-information");
        project_version_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");

        auto versionEffect = new QGraphicsOpacityEffect(this);
        versionEffect->setOpacity(0.5);

        project_version_label->setGraphicsEffect(versionEffect);
        project_version_label->setAutoFillBackground(true);

      
        project_last_edited_label = new QLabel(data.ProjectLastEdited, parent);
        project_last_edited_label->setGeometry(maxWidth, 35, 125, 20);
        project_last_edited_label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
        project_last_edited_label->setObjectName("project-information");
        project_last_edited_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");

        auto lastEditedEffect = new QGraphicsOpacityEffect(this);
        lastEditedEffect->setOpacity(0.5);

        project_last_edited_label->setGraphicsEffect(lastEditedEffect);
        project_last_edited_label->setAutoFillBackground(true);

        setContextMenuPolicy(Qt::CustomContextMenu);

        layout.addWidget(project_version_icon);
        layout.addWidget(project_name_label);
        layout.addWidget(project_directory_label);
        layout.addWidget(project_version_label);
        layout.addWidget(project_last_edited_label);
        setLayout(layout.layout());
    }

    QSize ProjectListItem::minimumSizeHint() const
    {
        return QSize(125, 55);
    }

    QString ProjectListItem::toCamelCase(const QString& s)
    {
        QStringList parts = s.split(' ', QString::SkipEmptyParts);
        for (int i = 0; i < parts.size(); ++i)
            parts[i].replace(0, 1, parts[i][0].toUpper());

        return parts.join(" ");
    }
}