// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LightEditor.hpp"
#include <noggit/DBC.h>
#include <format>
// #include <iostream>
#include <string>
#include <map>
#include "LightColorEditor.h"
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFormLayout>
// #include <QtWidgets/QLabel>
#include <QtWidgets/qtreewidget.h>

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>

using namespace Noggit::Ui::Tools;

LightEditor::LightEditor(MapView* map_view, QWidget* parent)
: QWidget(parent)
, _map_view(map_view)
, _world(map_view->getWorld())
{
	// auto Skies = _map_view->getWorld()->renderer()->skies()->skies;
	// Sky CurrSky = Skies[0];
	// get curent sky from camera position
	// Sky* CurrSky = _map_view->getWorld()->renderer()->skies()->findSkyWeights(map_view->getCamera()->position);

	setMinimumWidth(250);
	setMaximumWidth(250);

	auto layout(new QVBoxLayout(this));
	layout->setAlignment(Qt::AlignTop);



	auto lightning_tabs = new QTabWidget(this);
	layout->addWidget(lightning_tabs);

	auto light_selection_widget = new QWidget(this);
	light_selection_widget->setContentsMargins(0, 0, 0, 0);
	auto light_selection_layout = new QFormLayout(light_selection_widget); // QFormLayout
	light_selection_layout->setContentsMargins(0, 0, 0, 0);

	lightning_tabs->addTab(light_selection_widget, "Light Selection");


	auto light_editing_widget = new QWidget(lightning_tabs);
	light_editing_widget->setEnabled(false);
	auto light_editing_layout = new QVBoxLayout(light_editing_widget);
	lightning_tabs->addTab(light_editing_widget, "Edit Light");

	// set time _world->time
	light_selection_layout->addWidget(new QLabel("Set current time :", this));
	auto time_dial = new QDial(this);
	light_selection_layout->addWidget(time_dial);
	time_dial->setRange(0, 2880); // Time Values from 0 to 2880 where each number represents a half minute from midnight to midnight
	time_dial->setWrapping(true);
	time_dial->setSliderPosition(0); // to get ingame orientation
	// time_value0_dial->setValue(color0.time);
	time_dial->setInvertedAppearance(false); // sets the position at top
	time_dial->setToolTip("Time (24hours)");
	time_dial->setSingleStep(360); // ticks are 360 units

	QPushButton* GetCurrentSkyButton = new QPushButton("Edit current light", this);
	light_selection_layout->addWidget(GetCurrentSkyButton);



	_light_tree = new QTreeWidget();
	light_selection_layout->addWidget(_light_tree);
	_light_tree->setHeaderLabel("Current map lights");
	_light_tree->setColumnCount(1);
	// _light_tree->setMaximumHeight(400); // TODO : editing the height fucks up the layout

	// for (auto& sky : _world->renderer()->skies()->skies) // bad idea, renderer needs to be loaded first
	for (DBCFile::Iterator i = gLightDB.begin(); i != gLightDB.end(); ++i)
	{
		if (i->getInt(LightDB::Map) == _world->getMapID())
		{
			QTreeWidgetItem* item = new QTreeWidgetItem();

			std::stringstream ss;
			auto light_id = i->getUInt(LightDB::ID);
			item->setData(0, 0, QVariant(light_id) );

			std::string light_name = "Unamed Light";
			if (light_names_map.contains(light_id))
				light_name = light_names_map.at(light_id);

			else if (i->getFloat(LightDB::PositionX) == 0.0f && i->getFloat(LightDB::PositionY) == 0.0f && i->getFloat(LightDB::PositionZ) == 0.0f)
				light_name = "Global Light";

			ss << i->getUInt(LightDB::ID) << "-" << light_name;// gAreaDB.getAreaName(area_id);
			item->setText(0, QString(ss.str().c_str()));// TODO : light names
			// if (global)
			_light_tree->addTopLevelItem(item);
		}
	}

	QPushButton* GetSelectedSkyButton = new QPushButton("Edit selected light", this);
	light_selection_layout->addWidget(GetSelectedSkyButton);

	// global settings ********************************************************************************************** //
	// TODO : name lights on laoding instead
	light_editing_layout->addWidget(new QLabel("Selected Light :", this), 0, 0);
	auto lightid_label = new QLabel("No light selected", this);
	light_editing_layout->addWidget(lightid_label);

	QPushButton* SaveCurrentSkyButton = new QPushButton("Save Light(Write DBCs)", this);
	SaveCurrentSkyButton->setEnabled(false);
	light_editing_layout->addWidget(SaveCurrentSkyButton);

	QGroupBox* global_values_group = new QGroupBox("Global settings", this);
	// alpha_values_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	auto global_values_layout = new QGridLayout(global_values_group);

	global_values_layout->addWidget(new QLabel("Name:", this), 0, 0);
	auto name_line_edit = new QLineEdit(this);
	global_values_layout->addWidget(name_line_edit, 0, 1);

	global_values_layout->addWidget(new QLabel("Position X:", this),1,0);
	auto pos_x_spin = new QDoubleSpinBox(this);
	pos_x_spin->setRange(-17066.66656 * 2, 17066.66656*2); // size = ±17066.66656
	pos_x_spin->setValue(0);
	pos_x_spin->setSingleStep(50);
	pos_x_spin->setEnabled(false);
	global_values_layout->addWidget(pos_x_spin,1,1);

	global_values_layout->addWidget(new QLabel("Position Y:", this),2,0);
	auto pos_y_spin = new QDoubleSpinBox(this);
	pos_y_spin->setRange(-17066.66656 * 2, 17066.66656*2); // size = ±17066.66656
	pos_y_spin->setValue(0);
	pos_y_spin->setSingleStep(50);
	pos_y_spin->setEnabled(false);
	global_values_layout->addWidget(pos_y_spin,2,1);

	global_values_layout->addWidget(new QLabel("Position Z:", this),3,0);
	auto pos_z_spin = new QDoubleSpinBox(this);
	pos_z_spin->setRange(-17066.66656 * 2, 17066.66656*2); // ????
	pos_z_spin->setValue(0);
	pos_z_spin->setSingleStep(50);
	pos_z_spin->setEnabled(false);
	global_values_layout->addWidget(pos_z_spin,3,1);
	
	global_values_layout->addWidget(new QLabel("Inner Radius:", this),4,0);
	auto inner_radius_spin = new QDoubleSpinBox(this);
	inner_radius_spin->setRange(0, 100000); // max seen in dbc is 3871 (139363 ÷ 36 )
	inner_radius_spin->setValue(0);
	inner_radius_spin->setSingleStep(50);
	inner_radius_spin->setEnabled(false);
	global_values_layout->addWidget(inner_radius_spin,4,1);

	global_values_layout->addWidget(new QLabel("Outer Radius:", this),5,0);
	auto outer_radius_spin = new QDoubleSpinBox(this);
	outer_radius_spin->setRange(0, 100000); // max seen in dbc is 3871 (139363 ÷ 36 )
	outer_radius_spin->setValue(0);
	outer_radius_spin->setSingleStep(50);
	outer_radius_spin->setEnabled(false);
	global_values_layout->addWidget(outer_radius_spin,5,1);

	light_editing_layout->addWidget(global_values_group);

	// BELOW IS PARAM SPECIFIC SETTINGS
	light_editing_layout->addWidget(new QLabel("Param Type :", this));
	param_combobox = new QComboBox(this);
	param_combobox->setEnabled(false);
	light_editing_layout->addWidget(param_combobox);
	param_combobox->addItem("Clear Weather"); // Used in clear weather.
	param_combobox->addItem("Clear Weather Underwater"); // Used in clear weather while being underwater.
	param_combobox->addItem("Storm Weather"); // Used in rainy/snowy/sandstormy weather.
	param_combobox->addItem("Storm Weather Underwater"); // Used in rainy/snowy/sandstormy weather while being underwater.
	param_combobox->addItem("Death Effect"); // ParamsDeath. Only 4 and in newer ones 3 are used as value here (with some exceptions). Changing this seems to have no effect in 3.3.5a (is death light setting hardcoded?)
	// param_combobox->addItem("Clear Weather");
	// param_combobox->addItem("Clear Weather");
	// param_combobox->addItem("Clear Weather");

	// for (int i = 0; i < NUM_SkyParamsNames; ++i)
	// {
	// 	// auto sky_param = _curr_sky->skyParams[i];
	// }

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
	glow_slider->setValue(100);
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
			QSize(150, LIGHT_VIEW_PREVIEW_HEIGHT));
		LightsPreview.push_back(LightPrev);
		color_values_layout->addWidget(LightPrev, i, 0);

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
			_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880);
		}
	);

	connect(GetCurrentSkyButton, &QPushButton::clicked, [=]() {

		// _curr_sky = _map_view->getWorld()->renderer()->skies()->findSkyWeights(map_view->getCamera()->position);
		auto new_sky = _map_view->getWorld()->renderer()->skies()->findClosestSkyByWeight();
		if (_curr_sky == nullptr)
			return; // todo error
		else
			_curr_sky = new_sky;

		light_editing_widget->setEnabled(true);
		lightning_tabs->setCurrentWidget(light_editing_widget);

		SaveCurrentSkyButton->setEnabled(true);
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

		name_line_edit->setText(_curr_sky->name);
		pos_x_spin->setValue(_curr_sky->pos.x);
		pos_x_spin->setEnabled(true);
		pos_y_spin->setValue(_curr_sky->pos.z); // swap Z and Y
		pos_y_spin->setEnabled(true);
		pos_z_spin->setValue(_curr_sky->pos.y);
		pos_z_spin->setEnabled(true);
		inner_radius_spin->setValue(_curr_sky->r1);
		inner_radius_spin->setEnabled(true);
		outer_radius_spin->setValue(_curr_sky->r2);
		outer_radius_spin->setEnabled(true);

		param_combobox->setEnabled(true);
		param_combobox->setCurrentIndex(0);
		// light param specific data (only 0 currently)
		auto default_param = _curr_sky->skyParams[param_combobox->currentIndex()];
		glow_slider->setSliderPosition(default_param->glow() * 100);
		glow_slider->setEnabled(true);
		highlight_sky_checkbox->setCheckState(Qt::CheckState(default_param->highlight_sky() ) ); 
		highlight_sky_checkbox->setEnabled(true);
		if (_curr_sky->skybox.has_value())
			skybox_model_lineedit->setText(QString::fromStdString(default_param->skybox.value().model.get()->file_key().filepath()));
		// alpha values
		shallow_water_alpha_slider->setSliderPosition(default_param->river_shallow_alpha() * 100); // TODO bug : when reselecting the same light, sliders with digits values reset to 0.
		shallow_water_alpha_slider->setEnabled(true);
		deep_water_alpha_slider->setSliderPosition(default_param->river_deep_alpha() * 100);
		deep_water_alpha_slider->setEnabled(true);
		shallow_ocean_alpha_slider->setSliderPosition(default_param->ocean_shallow_alpha() * 100);
		shallow_ocean_alpha_slider->setEnabled(true);
		deep_ocean_alpha_slider->setSliderPosition(default_param->ocean_deep_alpha() * 100);
		deep_ocean_alpha_slider->setEnabled(true);
		// color values
		for (int i = 0; i < NUM_SkyColorNames; ++i)
		{
			
			LightsPreview[i]->SetPreview(default_param->colorRows[i]);

			//for (int l = 0; l < default_param->colorRows[i]; ++)
			//default_param->colorRows[i]
			//_color_value_Buttons[i]->setText(QString::fromStdString(std::format("{} / 16 values", default_param->colorRows[i].size())));
			//_color_value_Buttons[i]->setEnabled(true);
		}

		});

	connect(GetSelectedSkyButton, &QPushButton::clicked, [=]() {
		// TODO : load selected sky and teleport to it
		// auto selecetd_light = _light_tree->selectedItems().data

		auto const& selected_items = _light_tree->selectedItems();
		if (selected_items.size())
		{
			auto selected_light_id = selected_items.back()->data(0, 0).toInt(); // TODO : doesn't work since moving to role 0

			for (int i = 0; i < _map_view->getWorld()->renderer()->skies()->skies.size(); i++)
			{
				auto &sky = _map_view->getWorld()->renderer()->skies()->skies[i];
				if (sky.Id == selected_light_id)
				{
					_curr_sky = &sky;
					_map_view->_camera.position = _curr_sky->pos;
					_map_view->_camera.position.z += 100;
					// get terrain's height for Z axis.
					// auto chunk = _world->getChunkAt(glm::vec3(_curr_sky->pos.x, _curr_sky->pos.y, _curr_sky->pos.z)); // need to load tile first ??
					// if (chunk != nullptr)
					// 	_map_view->_camera.position.z = chunk->getMaxHeight() + 20.0f; 
					// TODO : initialise
				}
			}
		}
		});

	connect(param_combobox, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {

		// update rendering to selected param
		_curr_sky->curr_sky_param = index;
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky

		DBCFile::Record data = gLightDB.getByID(_curr_sky->Id);
		int nb_user = 0;
		for (DBCFile::Iterator i = gLightDB.begin(); i != gLightDB.end(); ++i)
		{
			for (int l = 0; l < NUM_SkyParamsNames; l++)
			{
				if (i->getInt(LightDB::DataIDs + l) == data.getInt(LightDB::DataIDs + index))
					nb_user++;
			}
		}
		_nb_param_users->setText("This param is used " + nb_user);

		auto sky_param = _curr_sky->skyParams[index];

		glow_slider->setSliderPosition(sky_param->glow() * 100);
		highlight_sky_checkbox->setCheckState(Qt::CheckState(sky_param->highlight_sky()));
		if (_curr_sky->skybox.has_value())
			skybox_model_lineedit->setText(QString::fromStdString(sky_param->skybox.value().model.get()->file_key().filepath()));
		// alpha values
		shallow_water_alpha_slider->setSliderPosition(sky_param->river_shallow_alpha() * 100);
		deep_water_alpha_slider->setSliderPosition(sky_param->river_deep_alpha() * 100);
		shallow_ocean_alpha_slider->setSliderPosition(sky_param->ocean_shallow_alpha() * 100);
		deep_ocean_alpha_slider->setSliderPosition(sky_param->ocean_deep_alpha() * 100);
		// color values
		for (int i = 0; i < NUM_SkyColorNames; ++i)
		{
			LightsPreview[i]->SetPreview(sky_param->colorRows[i]);
			//_color_value_Buttons[i]->setText(QString::fromStdString(std::format("{} / 16 values", sky_param->colorRows[i].size())));
		}
		});

	connect(SaveCurrentSkyButton, &QPushButton::clicked, [=]() {
		_curr_sky->save_to_dbc();
		});

	connect(pos_x_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->pos.x = v; // pos_x_spin->value();
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(pos_y_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->pos.z = v; // pos_y_spin->value();
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(pos_z_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->pos.y = v; //  pos_z_spin->value();
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(inner_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->r1 = v; //  inner_radius_spin->value();
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(outer_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		_curr_sky->r2 = v; //  outer_radius_spin->value();
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(glow_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_glow(v / 100); // glow_slider->value() / 100; // crashes, glow_slider is null.
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});
	
	connect(highlight_sky_checkbox, &QCheckBox::stateChanged, [&](int state) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_highlight_sky(state);
			_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(shallow_water_alpha_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_river_shallow_alpha(v / 100);
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(deep_water_alpha_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_river_deep_alpha(v / 100);
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(shallow_ocean_alpha_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_ocean_shallow_alpha(v / 100);
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

	connect(deep_ocean_alpha_slider, &QSlider::valueChanged, [&](int v) {
		_curr_sky->skyParams[param_combobox->currentIndex()]->set_ocean_deep_alpha(v / 100);
		_world->renderer()->skies()->update_sky_colors(_map_view->getCamera()->position, static_cast<int>(_world->time) % 2880); // find how to update sky
		});

}

void LightEditor::UpdateWorldTime()
{
	if (ActiveEditor.size() == 0)
		return;

	for (int i = 0; i < ActiveEditor.size(); ++i)
		ActiveEditor[i]->UpdateWorldTime();
}

void LightEditor::load_light_param()
{
	// _radius_slider->setValue(radius);

}

