// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LightEditor.hpp"
#include <noggit/DBC.h>
#include <format>
// #include <iostream>
#include <string>
#include <map>
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFormLayout>
// #include <QtWidgets/QLabel>
#include <QtWidgets/qtreewidget.h>
#include <noggit/ui/FontAwesome.hpp>

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>

using namespace Noggit::Ui::Tools;

LightEditor::LightEditor(MapView* map_view, QWidget* parent)
: QWidget(parent)
, _map_view(map_view)
, _world(map_view->getWorld())
{
	setMinimumWidth(250);
	setMaximumWidth(250);

	auto layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignTop);

	lightning_tabs = new QTabWidget(this);
	layout->addWidget(lightning_tabs);

	// light select tab
	auto light_selection_widget = new QWidget(lightning_tabs);
	auto light_selection_layout = new QVBoxLayout(light_selection_widget);
	light_selection_layout->setContentsMargins(0, 0, 0, 0);
	lightning_tabs->addTab(light_selection_widget, "Light Selection");

	// light edit tab
	_light_editing_widget = new QWidget(lightning_tabs);
	_light_editing_widget->setEnabled(false);
	auto light_editing_layout = new QVBoxLayout(_light_editing_widget);
	light_editing_layout->setContentsMargins(0, 0, 0, 0);
	lightning_tabs->addTab(_light_editing_widget, "Edit Light");


	// set time _world->time
	light_selection_layout->addWidget(new QLabel("Set current time :", this));
	auto time_dial = new QDial(this);
	light_selection_layout->addWidget(time_dial);
	time_dial->setRange(0, 2880); // Time Values from 0 to 2880 where each number represents a half minute from midnight to midnight
	time_dial->setWrapping(true);
	time_dial->setSliderPosition((int)_world->time); // to get ingame orientation
	// time_value0_dial->setValue(color0.time);
	time_dial->setInvertedAppearance(true); // sets the position at top
	time_dial->setToolTip("Time (24hours)");
	time_dial->setSingleStep(360); // ticks are 360 units (1/8 = 3 hours)

	QPushButton* GetCurrentSkyButton = new QPushButton("Edit current position's light", this);
	GetCurrentSkyButton->setToolTip("Selection the highest weight light at camera's position");
	GetCurrentSkyButton->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::cog));
	light_selection_layout->addWidget(GetCurrentSkyButton);


	light_selection_layout->addWidget(new QLabel("Current Map Lights :", this));

	_light_tree = new QListWidget(this);
	light_selection_layout->addWidget(_light_tree);
	// _light_tree->setWindowTitle("Current map lights");
	_light_tree->setViewMode(QListView::ListMode);
	_light_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	_light_tree->setSelectionBehavior(QAbstractItemView::SelectItems);
	_light_tree->setFixedHeight(580);
	_light_tree->setUniformItemSizes(true);

	// for (auto& sky : _world->renderer()->skies()->skies) // renderer needs to be loaded first
	for (DBCFile::Iterator i = gLightDB.begin(); i != gLightDB.end(); ++i)
	{
		if (i->getInt(LightDB::Map) == _world->getMapID())
		{
			QListWidgetItem* item = new QListWidgetItem();

			std::stringstream ss;
			unsigned int light_id = i->getUInt(LightDB::ID);
			item->setData(Qt::UserRole + 1, QVariant(light_id) ); // when setting an icon (global light), it uses role 1

			bool global = (i->getFloat(LightDB::PositionX) == 0.0f && i->getFloat(LightDB::PositionY) == 0.0f
							&& i->getFloat(LightDB::PositionZ) == 0.0f);

			std::string light_name = "Unamed Light";
			if (light_names_map.contains(light_id))
				light_name = light_names_map.at(light_id);
			else if (global)
				light_name = "Global Light";

			if (global)
			{
				item->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::sun));
			}

			ss << i->getUInt(LightDB::ID) << "-" << light_name;// gAreaDB.getAreaName(area_id);
			item->setText(QString(ss.str().c_str()));// TODO : light names
			// if (global)
			_light_tree->addItem(item);

			if (global)
			{
				_light_tree->setItemSelected(item, true);
			}
		}
	}

	QPushButton* GetSelectedSkyButton = new QPushButton("Edit selected light", this);
	GetSelectedSkyButton->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::cog));
	light_selection_layout->addWidget(GetSelectedSkyButton);

	QPushButton* addNewSkyButton = new QPushButton("(IN DEV) Duplicate selected(create new)", this);
	addNewSkyButton->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
	light_selection_layout->addWidget(addNewSkyButton);

	QPushButton* deleteSkyButton = new QPushButton("(IN DEV) delete light", this);
	deleteSkyButton->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::times));
	light_selection_layout->addWidget(deleteSkyButton);

	QPushButton* portToSkyButton = new QPushButton("(IN DEV) port to light", this);
	portToSkyButton->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::running));
	light_selection_layout->addWidget(portToSkyButton);

	light_selection_layout->addStretch();  // Add a stretch to prevent expansion

	// global settings ********************************************************************************************** //
	// TODO : name lights on laoding instead
	light_editing_layout->addWidget(new QLabel("Selected Light :", this), 0);
	lightid_label = new QLabel("No light selected", this);
	light_editing_layout->addWidget(lightid_label);

	save_current_sky_button = new QPushButton("Save Light(Write DBCs)", this);
	save_current_sky_button->setEnabled(false);
	light_editing_layout->addWidget(save_current_sky_button);

	QGroupBox* global_values_group = new QGroupBox("Global settings", this);
	// alpha_values_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	// auto global_values_layout = new QGridLayout(global_values_group);
	auto global_values_layout = new QFormLayout(global_values_group);

	name_line_edit = new QLineEdit(this);
	name_line_edit->setDisabled(true);
	global_values_layout->addRow("Name:", name_line_edit);

	global_light_chk = new QCheckBox("Global Light", this);
	global_light_chk->setToolTip("The map's global light will be used when the player isn't within any other light radius.");
	global_light_chk->setDisabled(true);
	global_values_layout->addRow(global_light_chk);

	pos_x_spin = new QDoubleSpinBox(this);
	pos_x_spin->setRange(-17066.66656 * 2, 17066.66656 * 2); // size = ±17066.66656
	pos_x_spin->setValue(0);
	pos_x_spin->setSingleStep(50);
	pos_x_spin->setEnabled(false);
	global_values_layout->addRow("Position X:", pos_x_spin);

	pos_y_spin = new QDoubleSpinBox(this);
	pos_y_spin->setRange(-17066.66656 * 2, 17066.66656 * 2); // size = ±17066.66656
	pos_y_spin->setValue(0);
	pos_y_spin->setSingleStep(50);
	pos_y_spin->setEnabled(false);
	global_values_layout->addRow("Position Y:", pos_y_spin);

	pos_z_spin = new QDoubleSpinBox(this);
	pos_z_spin->setRange(-17066.66656 * 2, 17066.66656*2); // ???? highest seen in 3.3.5 is 33,360
	pos_z_spin->setValue(0);
	pos_z_spin->setSingleStep(50);
	pos_z_spin->setEnabled(false);
	global_values_layout->addRow("Position Z:", pos_z_spin);
	
	inner_radius_spin = new QDoubleSpinBox(this);
	inner_radius_spin->setRange(0, 100000); // max seen in dbc is 3871 (139363 E36 )
	inner_radius_spin->setValue(0);
	inner_radius_spin->setSingleStep(50);
	inner_radius_spin->setEnabled(false);
	global_values_layout->addRow("Inner Radius:", inner_radius_spin);

	outer_radius_spin = new QDoubleSpinBox(this);
	outer_radius_spin->setRange(0, 100000); // max seen in dbc is 3871 (139363 E36 )
	outer_radius_spin->setValue(0);
	outer_radius_spin->setSingleStep(50);
	outer_radius_spin->setEnabled(false);
	global_values_layout->addRow("Outer Radius:", outer_radius_spin);

	light_editing_layout->addWidget(global_values_group);

	// BELOW IS PARAM SPECIFIC SETTINGS
	auto warning_label = new QLabel("Warning : Can't currently change param id,\n changes will affect all users of this param");
	warning_label->setStyleSheet("QLabel { color : orange; }");
	light_editing_layout->addWidget(warning_label);

	light_editing_layout->addWidget(new QLabel("Param Type :", this));
	param_combobox = new QComboBox(this);
	param_combobox->setEnabled(false);
	light_editing_layout->addWidget(param_combobox);
	// NUM_SkyParamsNames
	param_combobox->addItem("Clear Weather"); // Used in clear weather.
	param_combobox->addItem("Clear Weather Underwater"); // Used in clear weather while being underwater.
	param_combobox->addItem("Storm Weather"); // Used in rainy/snowy/sandstormy weather.
	param_combobox->addItem("Storm Weather Underwater"); // Used in rainy/snowy/sandstormy weather while being underwater.
	param_combobox->addItem("Death Effect"); // ParamsDeath. Only 4 and in newer ones 3 are used as value here (with some exceptions). Changing this seems to have no effect in 3.3.5a (is death light setting hardcoded?)
	param_combobox->addItem("unknown param 1");
	param_combobox->addItem("unknown param 2");
	param_combobox->addItem("unknown param 3");

	QGroupBox* light_param_group = new QGroupBox("Light Params", this);
	light_editing_layout->addWidget(light_param_group);
	auto light_param_layout = new QVBoxLayout(light_param_group);

	_nb_param_users = new QLabel(this);
	light_param_layout->addWidget(_nb_param_users);
	// QGroupBox* alpha_values_group = new QGroupBox("Alpha Values", this);
	// alpha_values_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	auto param_grid_layout = new QGridLayout(light_param_group);
	light_param_layout->addLayout(param_grid_layout);

	param_grid_layout->addWidget(new QLabel("Glow:", this),0,0);
	glow_slider = new QSlider(Qt::Horizontal, this);
	glow_slider->setRange(0, 100); // between 0 and 1, increases by 0.05. Multiplying everything by 100 cuz Qslider doesn't seem to support floats
	glow_slider->setTickInterval(5);
	glow_slider->setSingleStep(5);
	glow_slider->setValue(50);
	glow_slider->setEnabled(false);
	param_grid_layout->addWidget(glow_slider,0,1);

	param_grid_layout->addWidget(new QLabel("Highlight Sky:", this), 1, 0);
	highlight_sky_checkbox = new QCheckBox(this);
	highlight_sky_checkbox->setCheckState(Qt::Unchecked);
	highlight_sky_checkbox->setEnabled(false);
	param_grid_layout->addWidget(highlight_sky_checkbox, 1, 1);

	param_grid_layout->addWidget(new QLabel("Skybox model:", this), 2, 0);
	skybox_model_lineedit = new QLineEdit(this);
	skybox_model_lineedit->setEnabled(false);
	param_grid_layout->addWidget(skybox_model_lineedit, 2, 1);

	skybox_flag_1 = new QCheckBox("Full day Skybox", this);
	skybox_flag_1->setCheckState(Qt::Unchecked);
	skybox_flag_1->setEnabled(false);
	skybox_flag_1->setToolTip("animation syncs with time of day (uses animation 0, time of day is just in percentage).");
	param_grid_layout->addWidget(skybox_flag_1, 3, 0, 1, 2);
	skybox_flag_2 = new QCheckBox("Combine Procedural And Skybox", this);
	skybox_flag_2->setCheckState(Qt::Unchecked);
	skybox_flag_2->setEnabled(false);
	skybox_flag_2->setToolTip("render stars, sun and moons and clouds as well.");
	param_grid_layout->addWidget(skybox_flag_2, 4, 0, 1, 2);
	

	// Alpha values ********************************************************************************************** //

	QGroupBox* alpha_values_group = new QGroupBox("Alpha Values", light_param_group);	
	// light_editing_layout->addWidget(alpha_values_group);
	light_param_layout->addWidget(alpha_values_group);

	// alpha_values_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	auto alpha_values_layout = new QGridLayout(alpha_values_group);

	alpha_values_layout->addWidget(new QLabel("Shallow Water", alpha_values_group), 0, 0);
	shallow_water_alpha_slider = new QSlider(Qt::Horizontal, alpha_values_group);
	shallow_water_alpha_slider->setRange(0, 100); // between 0 and 1, increases by 0.05. Multiplying everything by 100 cuz Qslider doesn't seem to support floats
	shallow_water_alpha_slider->setTickInterval(5);
	shallow_water_alpha_slider->setSingleStep(5);
	shallow_water_alpha_slider->setValue(100);
	shallow_water_alpha_slider->setEnabled(false);
	alpha_values_layout->addWidget(shallow_water_alpha_slider, 0, 1);

	alpha_values_layout->addWidget(new QLabel("Deep Water", alpha_values_group), 1, 0);
	deep_water_alpha_slider = new QSlider(Qt::Horizontal, alpha_values_group);
	deep_water_alpha_slider->setRange(0, 100); // between 0 and 1, increases by 0.05. Multiplying everything by 100 cuz Qslider doesn't seem to support floats
	deep_water_alpha_slider->setTickInterval(5);
	deep_water_alpha_slider->setSingleStep(5);
	deep_water_alpha_slider->setValue(100);
	deep_water_alpha_slider->setEnabled(false);
	alpha_values_layout->addWidget(deep_water_alpha_slider, 1, 1);

	alpha_values_layout->addWidget(new QLabel("Shallow Ocean", alpha_values_group), 2, 0);
	shallow_ocean_alpha_slider = new QSlider(Qt::Horizontal, alpha_values_group);
	shallow_ocean_alpha_slider->setRange(0, 100); // between 0 and 1, increases by 0.05. Multiplying everything by 100 cuz Qslider doesn't seem to support floats
	shallow_ocean_alpha_slider->setTickInterval(5);
	shallow_ocean_alpha_slider->setSingleStep(5);
	shallow_ocean_alpha_slider->setValue(100);
	shallow_ocean_alpha_slider->setEnabled(false);
	alpha_values_layout->addWidget(shallow_ocean_alpha_slider, 2, 1);

	alpha_values_layout->addWidget(new QLabel("Deep Ocean", alpha_values_group), 3, 0);
	deep_ocean_alpha_slider = new QSlider(Qt::Horizontal, alpha_values_group);
	deep_ocean_alpha_slider->setRange(0, 100); // between 0 and 1, increases by 0.05. Multiplying everything by 100 cuz Qslider doesn't seem to support floats
	deep_ocean_alpha_slider->setTickInterval(5);
	deep_ocean_alpha_slider->setSingleStep(5);
	deep_ocean_alpha_slider->setValue(100);
	deep_ocean_alpha_slider->setEnabled(false);
	alpha_values_layout->addWidget(deep_ocean_alpha_slider, 3, 1);

	// Color values ********************************************************************************************** //
	QGroupBox* color_values_group = new QGroupBox("Light color values", this);
	// alpha_values_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	auto color_values_layout = new QGridLayout(color_values_group);

	for (int i = 0; i < NUM_SkyColorNames; ++i)
	{
		std::string color_name = sky_color_names_map.at(i);

		LightViewPreview* LightPrev = new LightViewPreview(QString("%1 Color").arg(color_name.c_str()),
			QSize(180, LIGHT_VIEW_PREVIEW_HEIGHT));
		LightsPreview.push_back(LightPrev);
		color_values_layout->addWidget(LightPrev, i, 0);

		int availableWidth = color_values_group->width() - color_values_layout->contentsMargins().left()
							- color_values_layout->contentsMargins().right() - color_values_layout->spacing();
		int test_suggestedWidth = LightPrev->sizeHint().width();
		int test_suggestedWidth2 = LightPrev->minimumSizeHint().width();

		connect(LightPrev, &LightViewPreview::LeftClicked, [this, i, LightPrev]()
			{
				if (!_curr_sky)
					return;

				LightViewEditor* Editor = new LightViewEditor(_map_view, _curr_sky->skyParams[param_combobox->currentIndex()], SkyColorNames(i), this);
				ActiveEditor.push_back(Editor);
				Editor->show();

				connect(Editor, &LightViewEditor::Delete, [=](LightViewEditor* self)
					{
						for (int i = 0; i < ActiveEditor.size(); ++i)
							if (ActiveEditor[i] == self)
								ActiveEditor.erase(ActiveEditor.begin() + i, ActiveEditor.begin() + i);
					});

				connect(Editor, &LightViewEditor::UpdatePixmap, [this, LightPrev](const QPixmap Updated)
					{
						LightPrev->UpdatePixmap(Updated);
					});
			});
	}

	light_editing_layout->addWidget(color_values_group);




	connect(time_dial, &QDial::valueChanged, [&](int v) // [this]
		{
			_map_view->getWorld()->time = v;
			_world->renderer()->skies()->force_update();
			UpdateWorldTime();
		}
	);

	connect(GetCurrentSkyButton, &QPushButton::clicked, [=]() {

		// Sky* new_sky = _map_view->getWorld()->renderer()->skies()->findSkyWeights(map_view->getCamera()->position); // this just returns the global sky
		// Sky* new_sky = _map_view->getWorld()->renderer()->skies()->findClosestSkyByDistance(map_view->getCamera()->position);
		Sky* new_sky = _map_view->getWorld()->renderer()->skies()->findClosestSkyByWeight();
		if (new_sky == nullptr)
			return; // todo error
		else
		{
			loadSelectSky(new_sky);
		}
		});



	connect(GetSelectedSkyButton, &QPushButton::clicked, [=]() {

		auto const& selected_items = _light_tree->selectedItems();
		if (selected_items.size())
		{
			unsigned int selected_light_id = selected_items.back()->data(Qt::UserRole + 1).toUInt();

			for (Sky& sky : _map_view->getWorld()->renderer()->skies()->skies)
			{
				if (sky.Id == selected_light_id)
				{
					loadSelectSky(&sky);

					// TODO : load tile and teleport where the light is at
					// 
					// _map_view->_camera.position = _curr_sky->pos;
					// _map_view->_camera.position.z += 100;
					// get terrain's height for Z axis.
					// auto chunk = _world->getChunkAt(glm::vec3(_curr_sky->pos.x, _curr_sky->pos.y, _curr_sky->pos.z)); // need to load tile first
					// if (chunk != nullptr)
					// _map_view->_camera.position.z = chunk->getMaxHeight() + 20.0f; 
					// TODO : initialize

					return;
				}
			}
		}
		});


	connect(_light_tree, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem* item)
		{
			unsigned int selected_light_id = item->data(Qt::UserRole + 1).toUInt();

			for (Sky& sky : _map_view->getWorld()->renderer()->skies()->skies)
			{
				if (sky.Id == selected_light_id)
				{
					loadSelectSky(&sky);

					return;
				}
			}
		});


	connect(addNewSkyButton, &QPushButton::clicked, [=]() {

		// TODO. when there is no light to duplicate we need create new function

		// _world->getRenderContext();

		// get selected sky to duplicate
		Sky* old_sky = nullptr;

		std::string old_name = "";

		auto const& selected_items = _light_tree->selectedItems();
		if (selected_items.size())
		{
			old_name = selected_items.back()->text().toStdString();
			unsigned int selected_light_id = selected_items.back()->data(Qt::UserRole + 1).toUInt();

			for (Sky& sky : _map_view->getWorld()->renderer()->skies()->skies)
			{
				if (sky.Id == selected_light_id)
				{
					old_sky = &sky;
				}
			}
		}
		if (old_sky == nullptr)
			return;

		if (old_sky->global)
		{
			QMessageBox::warning
			(nullptr
				, "Error"
				, "You cannot duplicate a Global light. "
				"There can only be one global light per map."
				, QMessageBox::Ok
			);
		}

		unsigned int new_light_id = gLightDB.getEmptyRecordID(LightDB::ID);

		// create new sky entry (duplicate)
		Sky* new_sky = _map_view->getWorld()->renderer()->skies()->createNewSky(old_sky, new_light_id, _map_view->getCamera()->position);

		// add new item to tree
		{
			QListWidgetItem* item = new QListWidgetItem();

			std::stringstream ss;
			item->setData(Qt::UserRole + 1, QVariant(new_light_id)); // when setting an icon (global light), it uses role 1

			std::string new_light_name = "Noggit Copy of " + old_name;

			ss << new_light_id << "-" << new_light_name;
			item->setText(QString(ss.str().c_str()));

			_light_tree->addItem(item);

			_light_tree->setCurrentItem(item);
			_light_tree->scrollToItem(item);
		}

		// loadSelectSky(new_sky);

		});

	connect(deleteSkyButton, &QPushButton::clicked, [=]() {

		});

	connect(portToSkyButton, &QPushButton::clicked, [=]() {

		});

	connect(param_combobox, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {

		// update rendering to selected param
		if (!_curr_sky)
		{
			assert(false);
			return;
		}

		if (!_curr_sky->skyParams[index])
			return;

		_curr_sky->curr_sky_param = index;
		_world->renderer()->skies()->force_update();

		load_light_param(index);

		});

	connect(save_current_sky_button, &QPushButton::clicked, [=]() {
		_curr_sky->save_to_dbc();
		});

	connect(pos_x_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->pos.x = v; // pos_x_spin->value();
		_world->renderer()->skies()->force_update();
		});

	connect(pos_y_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->pos.z = v; // pos_y_spin->value();
		_world->renderer()->skies()->force_update();
		});

	connect(pos_z_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->pos.y = v; //  pos_z_spin->value();
		_world->renderer()->skies()->force_update();
		});

	connect(inner_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->r1 = v; //  inner_radius_spin->value();
		_world->renderer()->skies()->force_update();
		});

	connect(outer_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->r2 = v; //  outer_radius_spin->value();
		_world->renderer()->skies()->force_update();
		});

	connect(glow_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_glow(v / 100);
		_world->renderer()->skies()->force_update();
		});
	
	connect(highlight_sky_checkbox, &QCheckBox::stateChanged, [&](int state) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_highlight_sky(state);
		_world->renderer()->skies()->force_update();
		});

	connect(shallow_water_alpha_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_river_shallow_alpha(v / 100);
		_world->renderer()->skies()->force_update();
		});

	connect(deep_water_alpha_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_river_deep_alpha(v / 100);
		_world->renderer()->skies()->force_update();
		});

	connect(shallow_ocean_alpha_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_ocean_shallow_alpha(v / 100);
		_world->renderer()->skies()->force_update();
		});

	connect(deep_ocean_alpha_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_ocean_deep_alpha(v / 100);
		_world->renderer()->skies()->force_update();
		});

	// connect(skybox_model_lineedit, &QLineEdit::textChanged, [&](std::string v) {
	QLineEdit::connect(skybox_model_lineedit, &QLineEdit::textChanged
	, [=]
	{
	auto text = skybox_model_lineedit->text().toStdString();
	if (text.empty())
	  _curr_sky->skyParams[param_combobox->currentIndex()]->skybox.reset();
	else
	_curr_sky->skyParams[param_combobox->currentIndex()]->skybox.emplace(text.c_str(), _world->getRenderContext());
	
	  _world->renderer()->skies()->force_update();
	});

	connect(skybox_flag_1, &QCheckBox::stateChanged, [&](int state) {
	if (state) // set bit
	  _curr_sky->skyParams[param_combobox->currentIndex()]->skyboxFlags |= (1 << (0));
	else // remove bit
	  _curr_sky->skyParams[param_combobox->currentIndex()]->skyboxFlags &= ~(1 << (0));
	});

	connect(skybox_flag_2, &QCheckBox::stateChanged, [&](int state) {
	if (state) // set bit
	  _curr_sky->skyParams[param_combobox->currentIndex()]->skyboxFlags |= (1 << (1));
	else // remove bit
	  _curr_sky->skyParams[param_combobox->currentIndex()]->skyboxFlags &= ~(1 << (1));
	});

	// connect(floats_editor_button, &QPushButton::clicked, [=]() {
	// LightFloatsEditor * Editor = new LightFloatsEditor(_map_view, _curr_sky->skyParams[param_combobox->currentIndex()], this);
	// 
	// 	Editor->show();
	// );

}

void LightEditor::loadSelectSky(Sky* sky)
{
	QSignalBlocker const _1(pos_x_spin);
	QSignalBlocker const _2(pos_y_spin);
	QSignalBlocker const _3(pos_z_spin);
	QSignalBlocker const _4(inner_radius_spin);
	QSignalBlocker const _5(outer_radius_spin);

	_curr_sky = sky;

	// disable combobox param items if param is not set or doesn't exist
	// in the future allow to add/edit param id
	for (int i = 0; i < NUM_SkyParamsNames; ++i)
	{
		auto sky_param = _curr_sky->skyParams[i];
		if (!sky_param)
		{
			QStandardItemModel* model = qobject_cast<QStandardItemModel*>(param_combobox->model());

			if (model) {
				QStandardItem* item = model->item(i);
				if (item) {
					item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
				}
			}
		}
	}

	_light_editing_widget->setEnabled(true);
	lightning_tabs->setCurrentWidget(_light_editing_widget);

	save_current_sky_button->setEnabled(true);
	// maybe move the inits to a separate function
	// global values
	std::stringstream ss;
	std::string light_name = "Unamed Light";
	if (light_names_map.contains(_curr_sky->Id))
		light_name = light_names_map.at(_curr_sky->Id);
	else if (_curr_sky->global)
		light_name = "Global Light";
	ss << _curr_sky->Id << "-" << light_name;
	lightid_label->setText(QString::fromStdString(ss.str().c_str()));

	// name_line_edit->setText(QString::fromStdString(_curr_sky->name));
	name_line_edit->setText(QString::fromStdString(light_name));

	if (_curr_sky->global)
	{
		global_light_chk->setChecked(true);
		pos_x_spin->setEnabled(false);
		pos_y_spin->setEnabled(false);
		pos_z_spin->setEnabled(false);
		inner_radius_spin->setEnabled(false);
		outer_radius_spin->setEnabled(false);
	}
	else
	{
		global_light_chk->setChecked(false);
		pos_x_spin->setEnabled(true);
		pos_y_spin->setEnabled(true);
		pos_z_spin->setEnabled(true);
		inner_radius_spin->setEnabled(true);
		outer_radius_spin->setEnabled(true);
	}

	pos_x_spin->setValue(_curr_sky->pos.x);
	pos_y_spin->setValue(_curr_sky->pos.z); // swap Z and Y
	pos_z_spin->setValue(_curr_sky->pos.y);
	inner_radius_spin->setValue(_curr_sky->r1);
	outer_radius_spin->setValue(_curr_sky->r2);

	param_combobox->setEnabled(true);

	{
	  QSignalBlocker const __(param_combobox);
	  param_combobox->setCurrentIndex(_curr_sky->curr_sky_param);
	}

	load_light_param(param_combobox->currentIndex());
}

void LightEditor::UpdateWorldTime()
{
	if (ActiveEditor.size() == 0)
		return;

	for (int i = 0; i < ActiveEditor.size(); ++i)
		ActiveEditor[i]->UpdateWorldTime();
}

void LightEditor::load_light_param(int param_id)
{
	if (!_curr_sky)
	{
		assert(false);
		return;
	}

	if (!_curr_sky->skyParams[param_id])
		return;

	// _curr_sky->curr_sky_param = param_id;
	// _world->renderer()->skies()->force_update();

	QSignalBlocker const _1(glow_slider);
	QSignalBlocker const _2(highlight_sky_checkbox);
	QSignalBlocker const _3(shallow_water_alpha_slider);
	QSignalBlocker const _4(deep_water_alpha_slider);
	QSignalBlocker const _5(shallow_ocean_alpha_slider);
	QSignalBlocker const _6(deep_ocean_alpha_slider);

	QSignalBlocker const _7(skybox_model_lineedit);
	QSignalBlocker const _8(skybox_flag_1);
	QSignalBlocker const _9(skybox_flag_2);
	
	auto sky_param = _curr_sky->skyParams[param_combobox->currentIndex()];

	int nb_user = 0;
	try
	{
		DBCFile::Record data = gLightDB.getByID(_curr_sky->Id);

		for (DBCFile::Iterator i = gLightDB.begin(); i != gLightDB.end(); ++i)
		{
			for (int l = 0; l < NUM_SkyParamsNames; l++)
			{
				if (i->getInt(LightDB::DataIDs + l) == data.getInt(LightDB::DataIDs + param_combobox->currentIndex()))
					nb_user++;
			}
		}
	}
	catch (...)
	{

	}
	
	std::stringstream ss;
	ss << "LightParam Id : " << sky_param->Id << "\nThis param is used " << nb_user << " times.";
	_nb_param_users->setText(QString::fromStdString(ss.str().c_str()));
	
	glow_slider->setSliderPosition(sky_param->glow() * 100);
	glow_slider->setEnabled(true);
	highlight_sky_checkbox->setCheckState(Qt::CheckState(sky_param->highlight_sky()));
	highlight_sky_checkbox->setEnabled(true);
		// alpha values
	shallow_water_alpha_slider->setSliderPosition(sky_param->river_shallow_alpha() * 100);
	shallow_water_alpha_slider->setEnabled(true);
	deep_water_alpha_slider->setSliderPosition(sky_param->river_deep_alpha() * 100);
	deep_water_alpha_slider->setEnabled(true);
	shallow_ocean_alpha_slider->setSliderPosition(sky_param->ocean_shallow_alpha() * 100);
	shallow_ocean_alpha_slider->setEnabled(true);
	deep_ocean_alpha_slider->setSliderPosition(sky_param->ocean_deep_alpha() * 100);
	deep_ocean_alpha_slider->setEnabled(true);
	// color values
	for (int i = 0; i < NUM_SkyColorNames; ++i)
	{
	  LightsPreview[i]->SetPreview(sky_param->colorRows[i]);
	  //_color_value_Buttons[i]->setText(QString::fromStdString(std::format("{} / 16 values", sky_param->colorRows[i].size())));
	}
	
	// skybox
	skybox_model_lineedit->setText("");
	if (sky_param->skybox.has_value())
	{
	  skybox_model_lineedit->setText(QString::fromStdString(sky_param->skybox.value().model.get()->file_key().filepath()));
	}
	skybox_model_lineedit->setEnabled(true);
	
	skybox_flag_1->setChecked(false);
	skybox_flag_2->setChecked(false);

	if (sky_param->skyboxFlags & (1 << 0))
	  skybox_flag_1->setChecked(true);

	if (sky_param->skyboxFlags & (1 << 1))
	  skybox_flag_1->setChecked(true);

	skybox_flag_1->setEnabled(true);
	skybox_flag_2->setEnabled(true);

}

