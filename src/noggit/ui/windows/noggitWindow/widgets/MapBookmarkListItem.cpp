#include <QListWidget>
#include <noggit/ui/windows/noggitWindow/widgets/MapBookmarkListItem.hpp>
#include <noggit/ui/FontAwesome.hpp>
#include <sstream>
namespace Noggit::Ui::Widget
{
    MapListBookmarkItem::MapListBookmarkItem(const MapListBookmarkData& data, QWidget* parent = nullptr) : QWidget(parent)
    {
        auto layout = QGridLayout();

        QIcon icon = FontAwesomeIcon(FontAwesome::bookmark);
      
        auto colour = new QGraphicsColorizeEffect(this);
        colour->setColor(QColor(255, 204, 0));
        colour->setStrength(1.0f);

        map_icon = new QLabel("", parent);
        map_icon->setPixmap(icon.pixmap(QSize(30, 30)));
        map_icon->setGeometry(0, 0, 32, 32);
        map_icon->setObjectName("project-icon-label");
        map_icon->setStyleSheet("QLabel#project-icon-label { font-size: 12px; padding: 0px;}");
        map_icon->setGraphicsEffect(colour);
        map_icon->setAutoFillBackground(true);

        auto projectName = toCamelCase(QString(data.MapName));
        map_name = new QLabel(projectName, parent);
        map_name->setGeometry(32, 0, 300, 20);
        map_name->setObjectName("project-title-label");
        map_name->setStyleSheet("QLabel#project-title-label { font-size: 12px; }");


        auto sstream = std::stringstream();
        sstream << std::to_string((int)data.Position.x) << " , " << std::to_string((int)data.Position.y) << " , " << std::to_string((int)data.Position.z);

        map_position = new QLabel(QString::fromStdString(sstream.str()), parent);
        map_position->setGeometry(32, 15, 300, 20);
        map_position->setObjectName("project-information");
        map_position->setStyleSheet("QLabel#project-information { font-size: 10px; }");

        auto directoryEffect = new QGraphicsOpacityEffect(this);
        directoryEffect->setOpacity(0.5);

        map_position->setGraphicsEffect(directoryEffect);
        map_position->setAutoFillBackground(true);

        setContextMenuPolicy(Qt::CustomContextMenu);

        layout.addWidget(map_icon);
        layout.addWidget(map_name);
        layout.addWidget(map_position);

        setLayout(layout.layout());
    }

    QSize MapListBookmarkItem::minimumSizeHint() const
    {
        return QSize(300, 32);
    }

    QString MapListBookmarkItem::toCamelCase(const QString& s)
    {
        QStringList parts = s.split(' ', Qt::SplitBehaviorFlags::SkipEmptyParts);
        for (int i = 0; i < parts.size(); ++i)
            parts[i].replace(0, 1, parts[i][0].toUpper());

        return parts.join(" ");
    }
}
