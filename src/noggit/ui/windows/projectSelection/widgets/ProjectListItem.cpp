#include <noggit/ui/windows/projectSelection/widgets/ProjectListItem.hpp>

namespace Noggit::Ui::Widget
{
  ProjectListItem::ProjectListItem(const ProjectListItemData& data, QWidget* parent = nullptr) : QWidget(parent)
  {
    auto layout = QGridLayout();

    QIcon icon;
    if (data.project_version == Project::ProjectVersion::WOTLK)
      icon = QIcon(":/icon-wrath");
    if (data.project_version == Project::ProjectVersion::SL)
      icon = QIcon(":/icon-shadow");
    _project_version_icon = new QLabel("", parent);
    _project_version_icon->setPixmap(icon.pixmap(QSize(48, 48)));
    _project_version_icon->setGeometry(0, 5, 64, 48);

    auto max_width = parent->sizeHint().width();

    auto project_name = toCamelCase(QString(data.project_name));
    _project_name_label = new QLabel(project_name, parent);
    _project_name_label->setGeometry(45, 5, max_width, 20);
    _project_name_label->setObjectName("project-title-label");
    _project_name_label->setStyleSheet("QLabel#project-title-label { font-size: 15px; }");

    _project_directory_label = new QLabel(data.project_directory, parent);
    _project_directory_label->setGeometry(48, 20, max_width, 20);
    _project_directory_label->setObjectName("project-information");
    _project_directory_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");
    _project_directory_label->setToolTip(data.project_directory);

    auto directory_effect = new QGraphicsOpacityEffect(this);
    directory_effect->setOpacity(0.5);

    _project_directory_label->setGraphicsEffect(directory_effect);
    _project_directory_label->setAutoFillBackground(true);

    QString version;
    if (data.project_version == Project::ProjectVersion::WOTLK)
      version = "Wrath Of The Lich King";
    if (data.project_version == Project::ProjectVersion::SL)
      version = "Shadowlands";

    _project_version_label = new QLabel(version, parent);
    _project_version_label->setGeometry(48, 35, max_width, 20);
    _project_version_label->setObjectName("project-information");
    _project_version_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");

    auto version_effect = new QGraphicsOpacityEffect(this);
    version_effect->setOpacity(0.5);

    _project_version_label->setGraphicsEffect(version_effect);
    _project_version_label->setAutoFillBackground(true);


    _project_last_edited_label = new QLabel(data.project_last_edited, parent);
    _project_last_edited_label->setGeometry(max_width, 35, 125, 20);
    _project_last_edited_label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    _project_last_edited_label->setObjectName("project-information");
    _project_last_edited_label->setStyleSheet("QLabel#project-information { font-size: 10px; }");

    auto last_edited_effect = new QGraphicsOpacityEffect(this);
    last_edited_effect->setOpacity(0.5);

    _project_last_edited_label->setGraphicsEffect(last_edited_effect);
    _project_last_edited_label->setAutoFillBackground(true);

    if (data.is_favorite)
    {
        _project_favorite_icon = new QLabel("", this);
        _project_favorite_icon->setPixmap(FontAwesomeIcon(FontAwesome::star).pixmap(QSize(16, 16)));
        _project_favorite_icon->setGeometry(max_width-10, 10, 125, 20);
        _project_favorite_icon->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
        _project_favorite_icon->setObjectName("project-favorite");
        _project_favorite_icon->setStyleSheet("QLabel#project-information { font-size: 10px; }");

        auto colour = new QGraphicsColorizeEffect(this);
        colour->setColor(QColor(255, 204, 0));
        colour->setStrength(1.0f);

        _project_favorite_icon->setGraphicsEffect(colour);
        _project_favorite_icon->setAutoFillBackground(true);

        layout.addWidget(_project_favorite_icon);
    }

    setContextMenuPolicy(Qt::CustomContextMenu);

    layout.addWidget(_project_version_icon);
    layout.addWidget(_project_name_label);
    layout.addWidget(_project_directory_label);
    layout.addWidget(_project_version_label);
    layout.addWidget(_project_last_edited_label);
    setLayout(layout.layout());
  }

  QSize ProjectListItem::minimumSizeHint() const
  {
    return QSize(125, 55);
  }

  QString ProjectListItem::toCamelCase(const QString& s)
  {
    QStringList parts = s.split(' ', Qt::SplitBehaviorFlags::SkipEmptyParts);
    for (int i = 0; i < parts.size(); ++i)
      parts[i].replace(0, 1, parts[i][0].toUpper());

    return parts.join(" ");
  }
}