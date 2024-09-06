#include "AssetBrowser.hpp"

#include <noggit/Log.h>
#include <noggit/ContextObject.hpp>
#include <noggit/ui/FramelessWindow.hpp>
#include <noggit/ui/FontNoggit.hpp>
#include <noggit/ui/GroundEffectsTool.hpp>
#include <noggit/MapView.h>
#include <noggit/ui/texturing_tool.hpp>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/project/CurrentProject.hpp>

#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QDir>
#include <QSettings>
#include <QPixmap>
#include <QIcon>
#include <QDialog>
#include <QDial>
#include <QSlider>

using namespace Noggit::Ui::Tools::AssetBrowser::Ui;
using namespace Noggit::Ui;

AssetBrowserWidget::AssetBrowserWidget(MapView* map_view, QWidget *parent)
: QMainWindow(parent, Qt::Window)
, _map_view(map_view)
{
  setWindowTitle("Asset Browser");

  auto body = new QWidget(this);
  ui = new ::Ui::AssetBrowser;
  ui->setupUi(body);
  setCentralWidget(body);

  setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowStaysOnTopHint);

  ui->checkBox_M2s->setChecked(true);
  ui->checkBox_WMOs->setChecked(true);
  connect(ui->checkBox_M2s, &QCheckBox::stateChanged, [&](int state)
      {
          updateModelData();
      });

  connect(ui->checkBox_WMOs, &QCheckBox::stateChanged, [&](int state)
      {
          updateModelData();
      });

  ui->comboBox_BrowseMode->addItems(brosweModeLabels.keys());

  // set combobox browse mode to World
  for (auto it = brosweModeLabels.constBegin(); it != brosweModeLabels.constEnd(); ++it) {
      if (it.value() == asset_browse_mode::world) {
          ui->comboBox_BrowseMode->setCurrentText(it.key());
          break;
      }
  }

  connect(ui->comboBox_BrowseMode, qOverload<int>(&QComboBox::currentIndexChanged)
      , [this](int index)
      {
          asset_browse_mode mode = brosweModeLabels[ui->comboBox_BrowseMode->currentText()];

          set_browse_mode(mode);
      }
  );

  _model = new QStandardItemModel(this);
  // _sort_model = new QSortFilterProxyModel(this);
  _sort_model = new NoggitExpendableFilterProxyModel;
  _sort_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
  _sort_model->setFilterRole(Qt::UserRole);
  _sort_model->setRecursiveFilteringEnabled(true);

  auto overlay = new QWidget(ui->viewport);
  viewport_overlay_ui = new ::Ui::AssetBrowserOverlay();
  viewport_overlay_ui->setupUi(overlay);
  overlay->setAttribute(Qt::WA_TranslucentBackground);
  overlay->setMouseTracking(true);
  overlay->setGeometry(0,0,ui->viewport->width(),ui->viewport->height());

  connect(ui->viewport, &ModelViewer::resized
      ,[this, overlay]()
      {
        overlay->setGeometry(0,0,ui->viewport->width(),ui->viewport->height());

        if (ui->viewport->width() < 700  && !overlay->isHidden())
          overlay->hide();
        else if (ui->viewport->width() > 700 && overlay->isHidden())
          overlay->setVisible(true);
      }
  );

  viewport_overlay_ui->toggleAnimationButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_ANIMATION));
  viewport_overlay_ui->toggleModelsButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_DOODADS));
  viewport_overlay_ui->toggleParticlesButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_PARTICLES));
  viewport_overlay_ui->toggleBoundingBoxButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_WITH_BOX));
  viewport_overlay_ui->toggleWMOButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_WMO));
  viewport_overlay_ui->toggleGridButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_LINES));

  ui->viewport->installEventFilter(overlay);
  overlay->show();

  ui->viewport->setLightDirection(120.0f, 60.0f);

  // drag'n'drop
  ui->listfileTree->setDragEnabled(true);
  ui->listfileTree->setDragDropMode(QAbstractItemView::DragOnly);

  ui->listfileTree->setIconSize(QSize(90, 90));

  _sort_model->setSourceModel(_model);
  ui->listfileTree->setModel(_sort_model);

  _preview_renderer = new PreviewRenderer(90, 90,
      Noggit::NoggitRenderContext::ASSET_BROWSER_PREVIEW, this);
  _preview_renderer->setVisible(false);

  // just to initialize context, ugly-ish
  _preview_renderer->setModelOffscreen("world/wmo/azeroth/buildings/human_farm/farm.wmo");
  _preview_renderer->renderToPixmap();

  connect(ui->listfileTree->selectionModel(), &QItemSelectionModel::selectionChanged
      ,[=] (const QItemSelection& selected, const QItemSelection& deselected)
        {
            for (auto const& index : selected.indexes())
            {
              auto path = index.data(Qt::UserRole).toString();
              if (path.endsWith(".m2") || path.endsWith(".wmo"))
              {
                auto str_path = path.toStdString();

                ui->viewport->setModel(str_path);
                _selected_path = str_path;

                emit selectionChanged(str_path);
              }
            }

        }

  );

  connect(ui->viewport, &ModelViewer::model_set
      , [=](const std::string& filename)
      {
        viewport_overlay_ui->doodadSetSelector->clear();
        viewport_overlay_ui->doodadSetSelector->insertItems(0, ui->viewport->getDoodadSetNames(filename));

        bool is_wmo = QString::fromStdString(filename).endsWith(".wmo");
        viewport_overlay_ui->doodadSetSelector->setVisible(is_wmo);
        viewport_overlay_ui->doodadSetLabel->setVisible(is_wmo);
      }
  );

  connect(ui->viewport, &ModelViewer::gl_data_unloaded,[=] () { emit gl_data_unloaded(); });

  // Handle preview rendering and drag
  connect(ui->listfileTree, &QTreeView::expanded
      ,[this] (const QModelIndex& index)
      {
        QSettings settings;
        bool render_preview = settings.value("assetBrowser/render_asset_preview").toBool();

        if (!render_preview)
          return;

        for (int i = 0; i != _sort_model->rowCount(index); ++i)
        {
          auto child = index.child(i, 0);
          auto path = child.data(Qt::UserRole).toString();
          if (path.endsWith(".wmo") || path.endsWith(".m2"))
          {
            _preview_renderer->setModelOffscreen(path.toStdString());

            auto preview_pixmap = _preview_renderer->renderToPixmap();
            auto item = _model->itemFromIndex(_sort_model->mapToSource(child));
            item->setIcon(QIcon(*preview_pixmap));
            item->setDragEnabled(true);
            item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
          }
        }
      }

  );

  setupConnectsCommon();

  _wmo_group_and_lod_regex = QRegularExpression(".+_\\d{3}(_lod.+)*.wmo");

  updateModelData();


}

void AssetBrowserWidget::setupConnectsCommon()
{
  connect(ui->searchButton, &QPushButton::clicked
      ,[this]()
          {
              _sort_model->setFilterFixedString(ui->searchField->text());
          }

  );

  connect(viewport_overlay_ui->lightDirY, &QDial::valueChanged
      ,[this]()
          {
              ui->viewport->setLightDirection(viewport_overlay_ui->lightDirY->value(),
                                              viewport_overlay_ui->lightDirZ->value());
          }
  );

  connect(viewport_overlay_ui->lightDirZ, &QSlider::valueChanged
      ,[this]()
          {
              ui->viewport->setLightDirection(viewport_overlay_ui->lightDirY->value(),
                                              viewport_overlay_ui->lightDirZ->value());
          }
  );

  connect(viewport_overlay_ui->moveSensitivitySlider, &QSlider::valueChanged
      ,[this]()
          {
              ui->viewport->setMoveSensitivity(static_cast<float>(viewport_overlay_ui->moveSensitivitySlider->value()));
          }
  );

  connect(ui->viewport, &ModelViewer::sensitivity_changed
      ,[this]()
          {
              viewport_overlay_ui->moveSensitivitySlider->setValue(ui->viewport->getMoveSensitivity() * 30.0f);
          }
  );

  connect(viewport_overlay_ui->cameraXButton, &QPushButton::clicked
      ,[this]()
          {
              ui->viewport->resetCamera(0.f, 0.f, 0.f, 0.f, -90.f, 0.f);
          }
  );

  connect(viewport_overlay_ui->cameraYButton, &QPushButton::clicked
      ,[this]()
          {
              ui->viewport->resetCamera(0.f, 0.f, 0.f, 0.f, 0, 90.f);
          }
  );

  connect(viewport_overlay_ui->cameraZButton, &QPushButton::clicked
      ,[this]()
          {
              ui->viewport->resetCamera(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
          }
  );

  connect(viewport_overlay_ui->cameraResetButton, &QPushButton::clicked
      ,[this]()
          {
              ui->viewport->resetCamera();
          }
  );

  connect(viewport_overlay_ui->doodadSetSelector, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
          {
              ui->viewport->setActiveDoodadSet(ui->viewport->getLastSelectedModel(),
                                               viewport_overlay_ui->doodadSetSelector->currentText().toStdString());
          }
  );

  // Render toggles
  connect(viewport_overlay_ui->toggleWMOButton, &QPushButton::clicked,
          [this]() {ui->viewport->_draw_wmo.toggle();});
  connect(viewport_overlay_ui->toggleBoundingBoxButton, &QPushButton::clicked,
          [this]() {ui->viewport->_draw_boxes.toggle();});
  connect(viewport_overlay_ui->toggleParticlesButton, &QPushButton::clicked,
          [this]() {ui->viewport->_draw_particles.toggle();});
  connect(viewport_overlay_ui->toggleModelsButton, &QPushButton::clicked,
          [this]() {ui->viewport->_draw_models.toggle();});
  connect(viewport_overlay_ui->toggleAnimationButton, &QPushButton::clicked,
          [this]() {ui->viewport->_draw_animated.toggle();});
  connect(viewport_overlay_ui->toggleGridButton, &QPushButton::clicked,
          [this]() {ui->viewport->_draw_grid.toggle();});

  connect(&ui->viewport->_draw_wmo, &BoolToggleProperty::changed,
          [this](bool state) {ui->viewport->_draw_wmo.set(state);});
  connect(&ui->viewport->_draw_boxes, &BoolToggleProperty::changed,
          [this](bool state) {ui->viewport->_draw_boxes.set(state);});
  connect(&ui->viewport->_draw_particles, &BoolToggleProperty::changed,
          [this](bool state) {ui->viewport->_draw_particles.set(state);});
  connect(&ui->viewport->_draw_models, &BoolToggleProperty::changed,
          [this](bool state) {ui->viewport->_draw_models.set(state);});
  connect(&ui->viewport->_draw_animated, &BoolToggleProperty::changed,
          [this](bool state) {ui->viewport->_draw_animated.set(state);});
  connect(&ui->viewport->_draw_grid, &BoolToggleProperty::changed,
          [this](bool state) {ui->viewport->_draw_grid.set(state);});
}

bool AssetBrowserWidget::validateBrowseMode(const QString& wow_file_path)
{
    // if (_browse_mode == asset_browse_mode::detail_doodads)
    // {
    //     _sort_model->setFilterFixedString("world/nodxt/detail");
    // }


    // TODO : do it in sort model instead?
    switch (_browse_mode)
    {
    case asset_browse_mode::ALL:
        return true;
    case asset_browse_mode::world:
    {
        if (wow_file_path.startsWith("World", Qt::CaseInsensitive))
            return true;
        return false;
    }
    case asset_browse_mode::detail_doodads:
    {
        if (wow_file_path.startsWith("world/nodxt/detail/", Qt::CaseInsensitive) 
            && wow_file_path.endsWith(".m2"))
            return true;
        return false;
    }
    case asset_browse_mode::skybox:
    {
        if (wow_file_path.startsWith("environments/stars", Qt::CaseInsensitive)
            && wow_file_path.endsWith(".m2"))
            return true;
        return false;
    }
    case asset_browse_mode::creatures:
    {
        if (wow_file_path.startsWith("creature", Qt::CaseInsensitive)
            && wow_file_path.endsWith(".m2"))
            return true;
        return false;
    }
    case asset_browse_mode::characters:
    {
        if (wow_file_path.startsWith("character", Qt::CaseInsensitive)
            && wow_file_path.endsWith(".m2"))
            return true;
        return false;
    }
    case asset_browse_mode::particles:
    {
        if (wow_file_path.startsWith("particles", Qt::CaseInsensitive)
            && wow_file_path.endsWith(".m2"))
            return true;
        return false;
    }
    case asset_browse_mode::cameras:
    {
        if (wow_file_path.startsWith("cameras", Qt::CaseInsensitive)
            && wow_file_path.endsWith(".m2"))
            return true;
        return false;
    }
    case asset_browse_mode::items:
    {
        if (wow_file_path.startsWith("item", Qt::CaseInsensitive)
            && wow_file_path.endsWith(".m2"))
            return true;
        return false;
    }
    case asset_browse_mode::spells:
    {
        if (wow_file_path.startsWith("SPELLS", Qt::CaseInsensitive)
            && wow_file_path.endsWith(".m2"))
            return true;
        return false;
    }
    default:
        return false; // ?
    }
}

// Add WMOs and M2s from project directory recursively
void AssetBrowserWidget::recurseDirectory(Model::TreeManager& tree_mgr, const QString& s_dir, const QString& project_dir)
{
  QDir dir(s_dir);
  QFileInfoList list = dir.entryInfoList();
  for (int i = 0; i < list.count(); ++i)
  {
    QFileInfo info = list[i];

    QString q_path = info.filePath();
    if (info.isDir())
    {
      if (info.fileName() != ".." && info.fileName() != ".")
      {
        recurseDirectory(tree_mgr, q_path, project_dir);
      }
    }
    else
    {
      if (!((q_path.endsWith(".wmo") && !_wmo_group_and_lod_regex.match(q_path).hasMatch())
      || q_path.endsWith(".m2")))
        continue;

      QString rel_path = QDir(project_dir).relativeFilePath(q_path.toStdString().c_str());

      if (!validateBrowseMode(rel_path))
          continue;

      tree_mgr.addItem(rel_path);
    }
  }
}

void AssetBrowserWidget::updateModelData()
{
  _model->clear();
  Model::TreeManager tree_mgr =  Model::TreeManager(_model);
  for (auto& key_pair : Noggit::Application::NoggitApplication::instance()->clientData()->listfile()->pathToFileDataIDMap())
  {
    std::string const& filename = key_pair.first;

    QString q_path = QString(filename.c_str());

    if (!( (ui->checkBox_WMOs->isChecked() && q_path.endsWith(".wmo")  && !_wmo_group_and_lod_regex.match(q_path).hasMatch())
        || (ui->checkBox_M2s->isChecked() && q_path.endsWith(".m2")) 
        ))
      continue;

    if (!validateBrowseMode(q_path))
        continue;

    tree_mgr.addItem(filename.c_str());
  }


  QSettings settings;
  QString project_dir = QString(Noggit::Project::CurrentProject::get()->ProjectPath.c_str());
  recurseDirectory(tree_mgr, project_dir, project_dir);

  _sort_model->setSortRole(Qt::UserRole);
  _sort_model->sort(0, Qt::AscendingOrder);

  // TODO : set default layout(base folder) for the modes
  if (_browse_mode != asset_browse_mode::ALL)
  {
      // expend base directories
      // ui->listfileTree->expand();
  }

  // those modes don't have subfolders, can auto expend without it becoming a mess
  if (_browse_mode == asset_browse_mode::detail_doodads
      || _browse_mode == asset_browse_mode::spells
      || _browse_mode == asset_browse_mode::skybox
      || _browse_mode == asset_browse_mode::cameras
      || _browse_mode == asset_browse_mode::particles)
  {
      ui->listfileTree->expandAll();
  }
}

AssetBrowserWidget::~AssetBrowserWidget()
{
  delete ui;
  delete _preview_renderer;
}

void AssetBrowserWidget::set_browse_mode(asset_browse_mode browse_mode)
{    
    if (_browse_mode == browse_mode)
        return;

    {
        QSignalBlocker const combobox_blocker(ui->comboBox_BrowseMode);

        QString key_text = "";

        for (auto it = brosweModeLabels.constBegin(); it != brosweModeLabels.constEnd(); ++it) {
            if (it.value() == browse_mode) {
                key_text = it.key();
                break; 
            }
        }
        // if (key_text.isEmpty())

        ui->comboBox_BrowseMode->setCurrentText(key_text);
    }

    _browse_mode = browse_mode;
    updateModelData();
}

void AssetBrowserWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
  {
    QString text = ui->searchField->text();
    _sort_model->setFilterFixedString(ui->searchField->text());
        ui->listfileTree->collapseAll();
    if (text.isEmpty() || text.length() < 3) // too few characters is too performance expensive, require 3
    {
        return;
    }

    // todo : crashes when typics something like creature

    ui->listfileTree->expandAll();

    // QList<QModelIndex> acceptedIndices = _sort_model->findIndices();
    // for (auto& index : acceptedIndices)
    // {
    //     if (!index.isValid())
    //         continue;
    //     ui->listfileTree->expand(index);
    // 
    //     auto expanderIndex = index.parent();
    //     while (expanderIndex.isValid())
    //     {
    //         if (!ui->listfileTree->isExpanded(expanderIndex))
    //         {
    //             ui->listfileTree->expand(expanderIndex);
    //             expanderIndex = expanderIndex.parent();
    //         }
    //         else
    //             break;
    //     }
    // }

  }
}
