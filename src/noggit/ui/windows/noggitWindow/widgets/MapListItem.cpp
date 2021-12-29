#include <QListWidget>
#include <noggit/ui/windows/noggitWindow/widgets/MapListItem.hpp>
#include <noggit/ui/FontAwesome.hpp>

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

        map_icon = new QLabel("", parent);
        map_icon->setPixmap(icon.pixmap(QSize(32, 32)));
        map_icon->setGeometry(0, 0, 32, 32);
        map_icon->setObjectName("project-icon-label");
        map_icon->setStyleSheet("QLabel#project-icon-label { font-size: 12px; padding: 0px;}");

        auto projectName = toCamelCase(QString(data.MapName));
        map_name = new QLabel(projectName, parent);
        map_name->setGeometry(32, 0, 300, 20);
        map_name->setObjectName("project-title-label");
        map_name->setStyleSheet("QLabel#project-title-label { font-size: 12px; }");

        map_id = new QLabel(QString::number(data.MapId), parent);
        map_id->setGeometry(32, 15, 300, 20);
        map_id->setObjectName("project-information");
        map_id->setStyleSheet("QLabel#project-information { font-size: 10px; }");

        auto directoryEffect = new QGraphicsOpacityEffect(this);
        directoryEffect->setOpacity(0.5);

        map_id->setGraphicsEffect(directoryEffect);
        map_id->setAutoFillBackground(true);

        auto instanceType = QString("Unknown");
        if (data.MapTypeId == 0)
            instanceType = QString("Continent");
        if (data.MapTypeId == 1)
            instanceType = QString("Dungeon");
        if (data.MapTypeId == 2)
            instanceType = QString("Raid");
        if (data.MapTypeId == 3)
            instanceType = QString("Battleground");
        if (data.MapTypeId == 4)
            instanceType = QString("Arena");
        if (data.MapTypeId == 5)
            instanceType = QString("Scenario");
      
        map_instance_type = new QLabel( instanceType,this);
        map_instance_type->setGeometry(150, 15, 125, 20);
        map_instance_type->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
        map_instance_type->setObjectName("project-information");
        map_instance_type->setStyleSheet("QLabel#project-information { font-size: 10px; }");

        auto lastEditedEffect = new QGraphicsOpacityEffect(this);
        lastEditedEffect->setOpacity(0.5);

        map_instance_type->setGraphicsEffect(lastEditedEffect);
        map_instance_type->setAutoFillBackground(true);

        if(data.Pinned)
        {
            map_pinned_label = new QLabel("", this);
            map_pinned_label->setPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16,16)));
            map_pinned_label->setGeometry(150, 0, 125, 20);
            map_pinned_label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            map_pinned_label->setObjectName("project-pinned");
            map_pinned_label->setStyleSheet("QLabel#project-pinned { font-size: 10px; }");

            auto colour = new QGraphicsColorizeEffect(this);
            colour->setColor(QColor(255,204,0));
            colour->setStrength(1.0f);

            map_pinned_label->setGraphicsEffect(colour);
            map_pinned_label->setAutoFillBackground(true);

            layout.addWidget(map_pinned_label);
        }

        setContextMenuPolicy(Qt::CustomContextMenu);

        layout.addWidget(map_icon);
        layout.addWidget(map_name);
        layout.addWidget(map_id);
        layout.addWidget(map_instance_type);
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
