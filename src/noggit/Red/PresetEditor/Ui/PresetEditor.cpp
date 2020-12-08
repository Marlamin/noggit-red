#include "PresetEditor.hpp"
#include <noggit/ui/FramelessWindow.hpp>
#include <noggit/ui/font_noggit.hpp>
#include <noggit/DBC.h>

#include <external/NodeEditor/include/nodes/NodeData>
#include <external/NodeEditor/include/nodes/FlowScene>
#include <external/NodeEditor/include/nodes/FlowView>
#include <external/NodeEditor/include/nodes/ConnectionStyle>
#include <external/NodeEditor/include/nodes/TypeConverter>
#include <noggit/Red/PresetEditor/Nodes/MathNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/ConditionNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/LogicIfNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/LogicBeginNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/LogicBreakNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/PrintNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/ForLoopNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/LogicChainNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/BaseNode.hpp>
#include <noggit/Red/PresetEditor/Nodes/Data/GenericTypeConverter.hpp>
#include <noggit/Red/PresetEditor/Nodes/Scene/NodeScene.hpp>


using namespace noggit::Red::PresetEditor::Ui;
using namespace noggit::ui;

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;
using QtNodes::ConnectionStyle;
using QtNodes::TypeConverter;
using QtNodes::TypeConverterId;

using noggit::Red::PresetEditor::Nodes::MathNode;
using noggit::Red::PresetEditor::Nodes::ConditionNode;
using noggit::Red::PresetEditor::Nodes::LogicIfNode;
using noggit::Red::PresetEditor::Nodes::LogicBeginNode;
using noggit::Red::PresetEditor::Nodes::PrintNode;
using noggit::Red::PresetEditor::Nodes::ForLoopNode;
using noggit::Red::PresetEditor::Nodes::LogicBreakNode;
using noggit::Red::PresetEditor::Nodes::LogicChainNode;
using noggit::Red::PresetEditor::Nodes::BaseNode;
using noggit::Red::PresetEditor::Nodes::NodeScene;

#define REGISTER_TYPE_CONVERTER(T_FROM, T_TO)             \
registerTypeConverter(std::make_pair(T_FROM##Data().type(), \
T_TO##Data().type()),                                     \
TypeConverter{T_FROM##To##T_TO##TypeConverter()})

static std::shared_ptr<DataModelRegistry>
registerDataModels()
{
  auto ret = std::make_shared<DataModelRegistry>();
 // ret->registerModel<NumberSourceDataModel>("I/O//Sources");

  //ret->registerModel<NumberDisplayDataModel>("I/O//Displays");

  ret->registerModel<MathNode>("Math");
  ret->registerModel<ConditionNode>("Logic");
  ret->registerModel<LogicBeginNode>("Logic//Flow");
  ret->registerModel<LogicIfNode>("Logic//Flow");
  ret->registerModel<ForLoopNode>("Logic//Flow");
  ret->registerModel<LogicBreakNode>("Logic//Flow");
  ret->registerModel<LogicChainNode>("Logic//Flow");
  ret->registerModel<PrintNode>("Functions//Generic");

  ret->REGISTER_TYPE_CONVERTER(Decimal, Integer);
  ret->REGISTER_TYPE_CONVERTER(Decimal, Boolean);
  ret->REGISTER_TYPE_CONVERTER(Integer, Decimal);
  ret->REGISTER_TYPE_CONVERTER(Integer, Boolean);
  ret->REGISTER_TYPE_CONVERTER(Boolean, Decimal);
  ret->REGISTER_TYPE_CONVERTER(Boolean, Integer);
  ret->REGISTER_TYPE_CONVERTER(Integer, String);
  ret->REGISTER_TYPE_CONVERTER(Decimal, String);

  return ret;
}


static
void
setStyle()
{
  ConnectionStyle::setConnectionStyle(
      R"(
    {
    "FlowViewStyle": {
      "BackgroundColor": [53, 53, 53],
      "FineGridColor": [60, 60, 60],
      "CoarseGridColor": [25, 25, 25]
    },
    "NodeStyle": {
      "NormalBoundaryColor": [255, 255, 255],
      "SelectedBoundaryColor": [255, 165, 0],
      "GradientColor0": "gray",
      "GradientColor1": [80, 80, 80],
      "GradientColor2": [64, 64, 64],
      "GradientColor3": [58, 58, 58],
      "ShadowColor": [20, 20, 20],
      "FontColor" : "white",
      "FontColorFaded" : "gray",
      "ConnectionPointColor": [169, 169, 169],
      "FilledConnectionPointColor": "cyan",
      "ErrorColor": "red",
      "WarningColor": [128, 128, 0],

      "PenWidth": 1.0,
      "HoveredPenWidth": 1.5,

      "ConnectionPointDiameter": 8.0,

      "Opacity": 0.8
    },
    "ConnectionStyle": {
      "ConstructionColor": "gray",
      "NormalColor": "darkcyan",
      "SelectedColor": [100, 100, 100],
      "SelectedHaloColor": "orange",
      "HoveredColor": "lightcyan",

      "LineWidth": 3.0,
      "ConstructionLineWidth": 2.0,
      "PointDiameter": 10.0,

      "UseDataDefinedColors": false
    }
  }
  )");
}

PresetEditorWidget::PresetEditorWidget(QWidget *parent)
: QMainWindow(parent, Qt::Window)
{
  setWindowTitle("Preset Editor");

  auto body = new QWidget(this);
  ui = new ::Ui::PresetEditor;
  ui->setupUi(body);
  setCentralWidget(body);

  auto titlebar = new QWidget(this);
  ui::setupFramelessWindow(titlebar, this, minimumSize(), maximumSize(), true);
  setMenuWidget(titlebar);

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

  viewport_overlay_ui->toggleAnimationButton->setIcon(font_noggit_icon(font_noggit::icons::VISIBILITY_ANIMATION));
  viewport_overlay_ui->toggleModelsButton->setIcon(font_noggit_icon(font_noggit::icons::VISIBILITY_DOODADS));
  viewport_overlay_ui->toggleParticlesButton->setIcon(font_noggit_icon(font_noggit::icons::VISIBILITY_UNUSED));
  viewport_overlay_ui->toggleBoundingBoxButton->setIcon(font_noggit_icon(font_noggit::icons::VISIBILITY_WITH_BOX));
  viewport_overlay_ui->toggleWMOButton->setIcon(font_noggit_icon(font_noggit::icons::VISIBILITY_WMO));
  viewport_overlay_ui->toggleGridButton->setIcon(font_noggit_icon(font_noggit::icons::VISIBILITY_LINES));

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
                                          noggit::NoggitRenderContext::PRESET_EDITOR_PREVIEW, this);
  _preview_renderer->setVisible(false);

  // just to initialize context, ugly-ish
  _preview_renderer->setModelOffscreen("world/wmo/azeroth/buildings/human_farm/farm.wmo");
  _preview_renderer->renderToPixmap();

  setupConnectsCommon();

  // Fill selector combo
  ui->worldSelector->addItem("None");
  ui->worldSelector->setItemData(0, QVariant(-1));
  int count = 1;
  for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
  {
    int map_id = i->getInt(MapDB::MapID);
    std::string name = i->getLocalizedString(MapDB::Name);
    int area_type = i->getUInt(MapDB::AreaType);

    if (area_type < 0 || area_type > 4 || !World::IsEditableWorld(map_id))
      continue;

    ui->worldSelector->addItem(QString::number(map_id) + " - " + QString::fromUtf8 (name.c_str()));
    ui->worldSelector->setItemData(count, QVariant(map_id), Qt::UserRole);

    auto map_internal_name = i->getString(MapDB::InternalName);
    ui->worldSelector->setItemData(count, QVariant(QString::fromStdString(map_internal_name)), Qt::UserRole + 1);

    count++;

  }

  // Handle minimap widget
  ui->minimapWidget->draw_boundaries(true);
  ui->minimapWidget->camera(ui->viewport->getWorldCamera());

  // Handles nodes
  ::setStyle();
  auto nodes_tab = ui->editorTabs->widget(1);
  auto scene = new NodeScene(::registerDataModels(), nodes_tab);
  nodes_tab->setLayout(new QVBoxLayout(nodes_tab));
  nodes_tab->layout()->addWidget(new FlowView(scene));
  nodes_tab->layout()->setContentsMargins(0, 0, 0, 0);
  nodes_tab->layout()->setSpacing(0);
  nodes_tab->resize(800, 600);

  connect(ui->executeButton, &QPushButton::clicked
    , [this, scene]()
    {
      scene->execute();
    });

  connect(ui->loadButton, &QPushButton::clicked
      , [this, scene]()
          {
            scene->load();
          });

  connect(ui->saveButton, &QPushButton::clicked
      , [this, scene]()
          {
              scene->save();
          });

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
      , [this] (::math::vector_3d const& pos)
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

  connect(&ui->viewport->_draw_wmo, &bool_toggle_property::changed,
          [this](bool state) {ui->viewport->_draw_wmo.set(state);});
  connect(&ui->viewport->_draw_boxes, &bool_toggle_property::changed,
          [this](bool state) {ui->viewport->_draw_boxes.set(state);});
  connect(&ui->viewport->_draw_particles, &bool_toggle_property::changed,
          [this](bool state) {ui->viewport->_draw_particles.set(state);});
  connect(&ui->viewport->_draw_models, &bool_toggle_property::changed,
          [this](bool state) {ui->viewport->_draw_models.set(state);});
  connect(&ui->viewport->_draw_animated, &bool_toggle_property::changed,
          [this](bool state) {ui->viewport->_draw_animated.set(state);});
  connect(&ui->viewport->_draw_grid, &bool_toggle_property::changed,
          [this](bool state) {ui->viewport->_draw_grid.set(state);});
}

PresetEditorWidget::~PresetEditorWidget()
{

}