#include <QListWidget>
#include <noggit/ui/windows/noggitWindow/widgets/MapListItem.hpp>
#include <noggit/ui/FontAwesome.hpp>

namespace Noggit::Ui::Widget
{
  MapListItem::MapListItem(const MapListData& data, QWidget* parent = nullptr)
    : QWidget(parent)
    , _map_data(data)
  {
    auto layout = QGridLayout();

    QIcon icon;
    switch (_map_data.expansion_id)
    {
      case 0: icon = QIcon(":/icon-classic"); break;
      case 1: icon = QIcon(":/icon-burning"); break;
      case 2: icon = QIcon(":/icon-wrath"); break;
      case 3: icon = QIcon(":/icon-cata"); break;
      case 4: icon = QIcon(":/icon-panda"); break;
      case 5: icon = QIcon(":/icon-warlords"); break;
      case 6: icon = QIcon(":/icon-legion"); break;
      case 7: icon = QIcon(":/icon-battle"); break;
      case 8: icon = QIcon(":/icon-shadow"); break;
      default: break;
    }

    _map_icon = new QLabel("", parent);
    _map_icon->setPixmap(icon.pixmap(QSize(32, 32)));
    _map_icon->setGeometry(0, 0, 32, 32);
    _map_icon->setObjectName("project-icon-label");
    _map_icon->setStyleSheet("QLabel#project-icon-label { font-size: 12px; padding: 0px;}");

    auto project_name = toCamelCase(QString(_map_data.map_name));
    _map_name = new QLabel(project_name, parent);
    _map_name->setGeometry(32, 0, 300, 20);
    _map_name->setObjectName("project-title-label");
    _map_name->setStyleSheet("QLabel#project-title-label { font-size: 12px; }");

    _map_id = new QLabel(QString::number(_map_data.map_id), parent);
    _map_id->setGeometry(32, 15, 300, 20);
    _map_id->setObjectName("project-information");
    _map_id->setStyleSheet("QLabel#project-information { font-size: 10px; }");

    auto directory_effect = new QGraphicsOpacityEffect(this);
    directory_effect->setOpacity(0.5);

    _map_id->setGraphicsEffect(directory_effect);
    _map_id->setAutoFillBackground(true);

    auto instance_type = QString("Unknown");
    switch (_map_data.map_type_id)
    {
      case 0: instance_type = "Continent"; break;
      case 1: instance_type = "Dungeon"; break;
      case 2: instance_type = "Raid"; break;
      case 3: instance_type = "Battleground"; break;
      case 4: instance_type = "Arena"; break;
      case 5: instance_type = "Scenario"; break;
      default: instance_type = "Unknown"; break;
    }

    _map_instance_type = new QLabel(instance_type, this);
    _map_instance_type->setGeometry(150, 15, 125, 20);
    _map_instance_type->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    _map_instance_type->setObjectName("project-information");
    _map_instance_type->setStyleSheet("QLabel#project-information { font-size: 10px; }");

    auto last_edited_effect = new QGraphicsOpacityEffect(this);
    last_edited_effect->setOpacity(0.5);

    _map_instance_type->setGraphicsEffect(last_edited_effect);
    _map_instance_type->setAutoFillBackground(true);

    if (_map_data.pinned)
    {
      _map_pinned_label = new QLabel("", this);
      _map_pinned_label->setPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16, 16)));
      _map_pinned_label->setGeometry(150, 0, 125, 20);
      _map_pinned_label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
      _map_pinned_label->setObjectName("project-pinned");
      _map_pinned_label->setStyleSheet("QLabel#project-pinned { font-size: 10px; }");

      auto colour = new QGraphicsColorizeEffect(this);
      colour->setColor(QColor(255, 204, 0));
      colour->setStrength(1.0f);

      _map_pinned_label->setGraphicsEffect(colour);
      _map_pinned_label->setAutoFillBackground(true);

      layout.addWidget(_map_pinned_label);
    }

    setContextMenuPolicy(Qt::CustomContextMenu);

    layout.addWidget(_map_icon);
    layout.addWidget(_map_name);
    layout.addWidget(_map_id);
    layout.addWidget(_map_instance_type);
    setLayout(layout.layout());
  }

  QSize MapListItem::minimumSizeHint() const
  {
    return QSize(300, 32);
  }

  QString MapListItem::toCamelCase(const QString& s)
  {
    QStringList parts = s.split(' ', Qt::SplitBehaviorFlags::SkipEmptyParts);
    for (int i = 0; i < parts.size(); ++i)
      parts[i].replace(0, 1, parts[i][0].toUpper());

    return parts.join(" ");
  }
}
