#include <noggit/ui/windows/noggitWindow/widgets/MapListItem.hpp>

namespace Noggit::Ui::Widget
{
    MapListItem::MapListItem(const MapListData& data, QWidget* parent = nullptr) : QWidget(parent)
    {
        auto layout = QGridLayout();

        QIcon icon;
        if (data.ExpansionId == 0)
            icon = QIcon(":/icon-classic");
        if (data.ExpansionId == 1)
             icon = QIcon(":/icon-burning");
        if (data.ExpansionId == 2)
             icon = QIcon(":/icon-wrath");
        if (data.ExpansionId == 3)
             icon = QIcon(":/icon-cata");
        if (data.ExpansionId == 4)
             icon = QIcon(":/icon-panda");
        if (data.ExpansionId == 5)
             icon = QIcon(":/icon-warlords");
        if (data.ExpansionId == 6)
             icon = QIcon(":/icon-legion");
        if (data.ExpansionId == 7)
             icon = QIcon(":/icon-battle");
        if (data.ExpansionId == 8)
             icon = QIcon(":/icon-shadow");

        project_version_icon = new QLabel("", parent);
        project_version_icon->setPixmap(icon.pixmap(QSize(32, 32)));
        project_version_icon->setGeometry(0, 0, 32, 32);
        project_version_icon->setObjectName("project-icon-label");
        project_version_icon->setStyleSheet("QLabel#project-icon-label { font-size: 12px; padding: 0px;}");

        auto projectName = toCamelCase(QString(data.MapName));
        project_name_label = new QLabel(projectName, parent);
        project_name_label->setGeometry(32, 0, 300, 20);
        project_name_label->setObjectName("project-title-label");
        project_name_label->setStyleSheet("QLabel#project-title-label { font-size: 12px; }");

        project_directory_label = new QLabel(QString::number(data.MapId), parent);
        project_directory_label->setGeometry(32, 15, 300, 20);
        project_directory_label->setObjectName("project-information");
        project_directory_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");

        auto directoryEffect = new QGraphicsOpacityEffect(this);
        directoryEffect->setOpacity(0.5);

        auto instanceType = QString("Unknown");
        if(data.MapTypeId == 0)
            instanceType = QString("Continent");
        if(data.MapTypeId == 1)
            instanceType = QString("Dungeon");
    	if(data.MapTypeId == 2)
    		instanceType = QString("Raid");
        if(data.MapTypeId == 3)
            instanceType = QString("Battleground");
        if(data.MapTypeId == 4)
            instanceType = QString("Arena");
        if(data.MapTypeId == 5)
            instanceType = QString("Scenario");

        project_directory_label->setGraphicsEffect(directoryEffect);
        project_directory_label->setAutoFillBackground(true);
      
        project_last_edited_label = new QLabel( instanceType,this);
        project_last_edited_label->setGeometry(150, 15, 125, 20);
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
        layout.addWidget(project_last_edited_label);
        setLayout(layout.layout());
    }

    QSize MapListItem::minimumSizeHint() const
    {
        return QSize(300, 32);
    }

    QString MapListItem::toCamelCase(const QString& s)
    {
        QStringList parts = s.split(' ', QString::SkipEmptyParts);
        for (int i = 0; i < parts.size(); ++i)
            parts[i].replace(0, 1, parts[i][0].toUpper());

        return parts.join(" ");
    }
}