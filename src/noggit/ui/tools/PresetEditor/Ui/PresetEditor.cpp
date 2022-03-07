#include "PresetEditor.hpp"
#include <noggit/ui/FramelessWindow.hpp>
#include <noggit/ui/FontNoggit.hpp>
#include <noggit/DBC.h>
#include <noggit/application/Utils.hpp>

using namespace Noggit::Ui::Tools::PresetEditor::Ui;
using namespace Noggit::Ui;

PresetEditorWidget::PresetEditorWidget(std::shared_ptr<Project::NoggitProject> project, QWidget *parent)
: QMainWindow(parent, Qt::Window), _project(project)
{
  setWindowTitle("Preset Editor");

  auto body = new QWidget(this);
  ui = new ::Ui::PresetEditor;
  ui->setupUi(body);
  setCentralWidget(body);

  setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowStaysOnTopHint);

  _model = new QFileSystemModel(this);
  _sort_model = new QSortFilterProxyModel(this);
  _sort_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
  _sort_model->setFilterRole(Qt::UserRole);
  _sort_model->setRecursiveFilteringEnabled(true);

  auto overlay = new QWidget(ui->viewport);
  viewport_overlay_ui = new ::Ui::PresetEditorOverlay();
  viewport_overlay_ui->setupUi(overlay);
  overlay->setAttribute(Qt::WA_TranslucentBackground);
  overlay->setMouseTracking(true);
  overlay->setGeometry(0,0,ui->viewport->width(),ui->viewport->height());

  connect(ui->viewport, &ModelViewer::resized
      ,[this, overlay]()
          {
              overlay->setGeometry(0,0,ui->viewport->width(),ui->viewport->height());
          }
  );

  viewport_overlay_ui->toggleAnimationButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_ANIMATION));
  viewport_overlay_ui->toggleModelsButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_DOODADS));
  viewport_overlay_ui->toggleParticlesButton->setIcon(FontNoggitIcon(FontNoggit::Icons::VISIBILITY_UNUSED));
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
                                          Noggit::NoggitRenderContext::PRESET_EDITOR_PREVIEW, this);
  _preview_renderer->setVisible(false);

  // just to initialize context, ugly-ish
  _preview_renderer->setModelOffscreen("world/wmo/azeroth/buildings/human_farm/farm.wmo");
  _preview_renderer->renderToPixmap();

  setupConnectsCommon();

  // Fill selector combo
  ui->worldSelector->addItem("None");
  ui->worldSelector->setItemData(0, QVariant(-1));

  const auto& table = std::string("Map");
  auto mapTable = _project->ClientDatabase->LoadTable(table, readFileAsIMemStream);

  int count = 1;
  auto iterator = mapTable.Records();
  while (iterator.HasRecords())
  {
      auto record = iterator.Next();

      int map_id = record.RecordId;
      std::string name = record.Columns["MapName_lang"].Value;
      int area_type = std::stoi(record.Columns["InstanceType"].Value);

      if (area_type < 0 || area_type > 4 || !World::IsEditableWorld(record))
          continue;

      ui->worldSelector->addItem(QString::number(map_id) + " - " + QString::fromUtf8(name.c_str()));
      ui->worldSelector->setItemData(count, QVariant(map_id), Qt::UserRole);

      auto map_internal_name = record.Columns["Directory"].Value;
      ui->worldSelector->setItemData(count, QVariant(QString::fromStdString(map_internal_name)), Qt::UserRole + 1);

      count++;
  }
  _project->ClientDatabase->UnloadTable("map");


  // Handle minimap widget
  ui->minimapWidget->draw_boundaries(true);
  ui->minimapWidget->camera(ui->viewport->getWorldCamera());

}

void PresetEditorWidget::setupConnectsCommon()
{
  connect(ui->searchButton, &QPushButton::clicked
      ,[this]()
          {
              _sort_model->setFilterFixedString(ui->searchField->text());
          }

  );

  connect(ui->worldUnloadButton, &QPushButton::clicked
      ,[this]()
          {
              int map_id = ui->worldSelector->itemData(ui->worldSelector->currentIndex(), Qt::UserRole).toInt();

              ui->viewport->loadWorldUnderlay(
                ui->worldSelector->itemData(ui->worldSelector->currentIndex(), Qt::UserRole + 1).toString().toStdString(), map_id);
              ui->minimapWidget->world(ui->viewport->getWorld());
          }

  );

  connect(ui->minimapWidget, &minimap_widget::map_clicked
      , [this] (::glm::vec3 const& pos)
          {
              ui->viewport->getWorldCamera()->position = pos;
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

PresetEditorWidget::~PresetEditorWidget()
{

}