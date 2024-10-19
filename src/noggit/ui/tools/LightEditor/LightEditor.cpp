// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LightEditor.hpp"
#include <noggit/DBC.h>
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFormLayout>
// #include <QtWidgets/QLabel>
#include <QtWidgets/qtreewidget.h>
#include <noggit/ui/FontAwesome.hpp>
#include <noggit/application/NoggitApplication.hpp>

#include <format>
#include <string>
#include <map>

#include <QGridLayout>
#include <QFormLayout>
#include <QTreeWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QFile>
#include <QTextStream>
#include <QStringList>

using namespace Noggit::Ui::Tools;

LightEditor::LightEditor(MapView* map_view, QWidget* parent)
: QWidget(parent)
, _map_view(map_view)
, _world(map_view->getWorld())
{
	setMinimumWidth(250);
	// setMaximumWidth(250);

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

	QPushButton* lightningInfoDialogButton = new QPushButton("View Lightning Info", this);
	lightningInfoDialogButton->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::sun));
	light_selection_layout->addWidget(lightningInfoDialogButton);

	_lightning_info_dialog = new LightningInfoDialog(this, this);

	// TODO can save this as a nice reusable text+tooltip indicator label widget
	QLabel* active_lights_label = new QLabel("Active Lights :", this);
	// Create a separate label for the icon
	QLabel* active_lights_icon_label = new QLabel(this);
	QIcon infoIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
	QPixmap infoPixmap = infoIcon.pixmap(16, 16);
	active_lights_icon_label->setPixmap(infoPixmap);
	// Set the tooltip text
	active_lights_icon_label->setToolTip("Current active Lights at camera position."
												"\nThe global light is used when no other light is active, or blended with them when not within inner radius."
												"\nDouble Click a row to edit it.");
	// Add both labels to a horizontal layout
	QHBoxLayout* active_ligths_label_layout = new QHBoxLayout(light_selection_widget);
	// active_ligths_label_layout->setContentsMargins(0, 0, 0, 0);
	active_ligths_label_layout->addWidget(active_lights_icon_label);
	active_ligths_label_layout->addWidget(active_lights_label);
	active_ligths_label_layout->addStretch();

	light_selection_layout->addLayout(active_ligths_label_layout);

	// current active lights tree
	_active_lights_tree = new QListWidget(this);
	light_selection_layout->addWidget(_active_lights_tree);
	// _light_tree->setWindowTitle("Current map lights");
	_active_lights_tree->setViewMode(QListView::ListMode);
	_active_lights_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	_active_lights_tree->setSelectionBehavior(QAbstractItemView::SelectItems);
	_active_lights_tree->setFixedHeight(60);
	_active_lights_tree->setUniformItemSizes(true);
	// _active_lights_tree->setContextMenuPolicy(Qt::CustomContextMenu);

	// QPushButton* GetCurrentSkyButton = new QPushButton("Edit current position's light", this);
	// GetCurrentSkyButton->setToolTip("Selection the highest weight light at camera's position");
	// GetCurrentSkyButton->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::cog));
	// light_selection_layout->addWidget(GetCurrentSkyButton);

	// auto lightningBox = new ExpanderWidget(this);
	// auto lightningBox_content = new QWidget(this);
	// auto lightningBox_content_layout = new QFormLayout(lightningBox_content);
	// lightningBox_content_layout->setAlignment(Qt::AlignTop);
	// lightningBox->setExpanderTitle("Current Lightning");

// 	_highest_weight_sky_label = new QLabel("None/Not initialized", this);
// 	lightningBox_content_layout->addRow("Highest Weight Light", _highest_weight_sky_label);
// 
// 	// current colors preview
// 	for (int i = 0; i < NUM_SkyColorNames; ++i)
// 	{
// 		std::string color_name = sky_color_names_map.at(i);
// 
// 		_current_lightning_colors_labels[i] = new QLabel(this);
// 		QLabel* colorIconLabel = _current_lightning_colors_labels[i];
// 		QPixmap colorIcon(16, 16);  // Create a 16x16 px pixmap
// 		colorIcon.fill(Qt::transparent);  // Transparent background
// 
// 		// Use QPainter to draw a red square icon
// 		QPainter painter(&colorIcon);
// 		painter.setBrush(QBrush(Qt::black));  // Set the brush to red
// 		painter.setPen(Qt::NoPen);  // No border
// 		painter.drawRect(0, 0, 16, 16);  // Draw the square
// 		colorIconLabel->setPixmap(colorIcon);  // Set the pixmap
// 
// 		lightningBox_content_layout->addRow(color_name.c_str(), colorIconLabel);
// 	}
// 	// current float params preview
// 	for (int i = 0; i < NUM_SkyFloatParamsNames; ++i)
// 	{
// 		_current_lightning_floats_labels[i] = new QLabel("0", this);
// 
// 		lightningBox_content_layout->addRow(sky_float_values_names_map.at(i).c_str(), _current_lightning_floats_labels[i]);
// 	}
// 	// light params preview
// 	{
// 		_river_shallow_alpha_label_label = new QLabel("0", this);
// 		lightningBox_content_layout->addRow("Shallow Water Alpha", _river_shallow_alpha_label_label);
// 		_river_deep_alpha_label = new QLabel("0", this);
// 		lightningBox_content_layout->addRow("Deep Water Alpha", _river_deep_alpha_label);
// 		_ocean_shallow_alpha_label = new QLabel("0", this);
// 		lightningBox_content_layout->addRow("Shallow Ocean Alpha", _ocean_shallow_alpha_label);
// 		_ocean_deep_alpha_label = new QLabel("0", this);
// 		lightningBox_content_layout->addRow("Deep Water Alpha", _ocean_deep_alpha_label);
// 		_glow_label = new QLabel("0", this);
// 		lightningBox_content_layout->addRow("Glow", _glow_label);
// 		_highlight_label = new QLabel("0", this);
// 		lightningBox_content_layout->addRow("Highlight Sky", _highlight_label);
// 	}

	// lightningBox->addPage(lightningBox_content);
	// light_selection_layout->addWidget(lightningBox);

	light_selection_layout->addWidget(new QLabel("Current Map Lights :", this));


	QHBoxLayout* filter_tree_ledit_layout = new QHBoxLayout(light_selection_widget);
	filter_tree_ledit_layout->addWidget(new QLabel("Filter :", this));
	_light_tree_filter = new QLineEdit(this);
	filter_tree_ledit_layout->addWidget(_light_tree_filter);
	// filter_tree_ledit_layout->addStretch();
	light_selection_layout->addLayout(filter_tree_ledit_layout);

	_light_tree = new QListWidget(this);
	light_selection_layout->addWidget(_light_tree);
	// _light_tree->setWindowTitle("Current map lights");
	_light_tree->setViewMode(QListView::ListMode);
	_light_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	_light_tree->setSelectionBehavior(QAbstractItemView::SelectItems);
	_light_tree->setFixedHeight(580);
	_light_tree->setUniformItemSizes(true);


	// load name definitions from csv file
	std::string definitions_path = Noggit::Application::NoggitApplication::instance()->getConfiguration()->ApplicationNoggitDefinitionsPath
															 + "\\light_dbc_names.csv";
	QString qPath = QString::fromStdString(definitions_path);
	QFile file(qPath);

	bool found_definitions = false;
	// load light names definition from csv file.
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) 
	{
		found_definitions = true;

		QTextStream in(&file);

		// Skip the header line
		std::string headerLine = in.readLine().toStdString();
		assert(headerLine == "ID,Name"); // "ID,Name,mapid"

		while (!in.atEnd()) 
		{
			QString line = in.readLine();
			QStringList fields = line.split(',');

			// Ensure there are at least two fields (ID and Name)
			assert(fields.size() == 2);
			if (fields.size() < 2) {
				continue;
			}
			bool ok;
			/*
			int map_id = fields[2].toInt(&ok);
			// only load this map ?
			if (map_id != _world->getMapID())
				continue;
			*/

			int id = fields[0].toInt(&ok);
			assert(ok);
			std::string const map_name = fields[1].toStdString();

			if (map_name.empty())
				continue;

			light_names_map[id] = map_name;
		}
		file.close();
	}

	// Load Tree from dbc. Could potentially be done from _world->renderer()->skies()->skies but it needs to be loaded first
	for (DBCFile::Iterator i = gLightDB.begin(); i != gLightDB.end(); ++i)
	{
		if (i->getInt(LightDB::Map) == _world->getMapID())
		{
			QListWidgetItem* item = new QListWidgetItem();

			std::stringstream ss;
			unsigned int light_id = i->getUInt(LightDB::ID);
			item->setData(Qt::UserRole + 1, QVariant(light_id) );

			bool global = (i->getFloat(LightDB::PositionX) == 0.0f && i->getFloat(LightDB::PositionY) == 0.0f
							&& i->getFloat(LightDB::PositionZ) == 0.0f);

			std::string light_name = getLightName(light_id, global); // TODO light zone arg

			if (global)
			{
				item->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::sun));
			}

			ss << light_id << "-" << light_name;
			item->setText(QString(ss.str().c_str()));
			// if (global)
			_light_tree->addItem(item);

			if (global)
			{
				_light_tree->setCurrentItem(item);
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

	light_selection_layout->addStretch();

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

	QHBoxLayout* checkboxes_layout = new QHBoxLayout();

	global_light_chk = new QCheckBox("Global Light", this);
	QString global_light_tooltip_str = "Hint : The map's global light will be used when the player isn't within any other light radius."
																				 "\nThere can only be one g lobal Light per map"
																				 "\nGlobal Lights are defined by having X:0, Y:0, Z:0 coordinates";
	global_light_chk->setToolTip(global_light_tooltip_str);
	global_light_chk->setDisabled(true);
	// global_values_layout->addRow(global_light_chk);

	zone_light_chk = new QCheckBox("Zone Light", this);
	zone_light_chk->setToolTip("Hint : This light is used by a Zone Light (Polygon).");
	zone_light_chk->setDisabled(true);
	// global_values_layout->addRow(global_light_chk);
	checkboxes_layout->addWidget(global_light_chk);
	checkboxes_layout->addWidget(zone_light_chk);

	global_values_layout->addRow(checkboxes_layout);


	name_line_edit = new QLineEdit(this);
	// name_line_edit->setDisabled(true);
	// global_values_layout->addRow("Name:", name_line_edit);

	// Create a small button
	QPushButton* save_name_button = new QPushButton(this);  // You can set any text or icon for the button
	save_name_button->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::save));
	// small_button->setFixedSize(20, 20);  // Set a small size for the button
	save_name_button->setToolTip("Save Name to \"noggit-definitions\\light_dbc_names.csv\""
															 "\nBlizzard names are datamined up to AQ40."
															"NOT YET IMPLEMENTED");
	save_name_button->setEnabled(false);

	// Create an HBoxLayout to hold the QLineEdit and button together
	QHBoxLayout* name_layout = new QHBoxLayout();
	name_layout->addWidget(new QLabel("Name:"));
	name_layout->addWidget(name_line_edit);
	name_layout->addWidget(save_name_button);

	global_values_layout->addRow(name_layout);

	pos_x_spin = new QDoubleSpinBox(this);
	pos_x_spin->setRange(-17066.66656 * 2, 17066.66656 * 2); // size = �17066.66656
	pos_x_spin->setValue(0);
	pos_x_spin->setSingleStep(50);
	pos_x_spin->setEnabled(false);
	global_values_layout->addRow("Position X:", pos_x_spin);

	pos_y_spin = new QDoubleSpinBox(this);
	pos_y_spin->setRange(-17066.66656 * 2, 17066.66656 * 2); // size = �17066.66656
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
	inner_radius_spin->setRange(0, 100000); // max seen in dbc is 3871 (139363 �E36 )
	inner_radius_spin->setValue(0);
	inner_radius_spin->setSingleStep(50);
	inner_radius_spin->setEnabled(false);
	global_values_layout->addRow("Inner Radius:", inner_radius_spin);

	outer_radius_spin = new QDoubleSpinBox(this);
	outer_radius_spin->setRange(0, 100000); // max seen in dbc is 3871 (139363 �E36 )
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
				Sky* curr_sky = get_selected_sky();

				if (!curr_sky)
					return;
				if (!curr_sky->getParam(param_combobox->currentIndex()))
					return;

				LightViewEditor* Editor = new LightViewEditor(_map_view, curr_sky->getParam(param_combobox->currentIndex()).value(), SkyColorNames(i), this);
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


	connect(lightningInfoDialogButton, &QPushButton::clicked, [=]() {

		_lightning_info_dialog->show();
		});

	connect(_active_lights_tree, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem* item)
		{
			unsigned int selected_light_id = item->data(Qt::UserRole + 1).toUInt();

			Sky* sky = _map_view->getWorld()->renderer()->skies()->findSkyById(selected_light_id);
			if (sky)
				loadSelectSky(sky);
		});


	// connect(GetCurrentSkyButton, &QPushButton::clicked, [=]() {
	// 
	// 	// Sky* new_sky = _map_view->getWorld()->renderer()->skies()->findSkyWeights(map_view->getCamera()->position); // this just returns the global sky
	// 	// Sky* new_sky = _map_view->getWorld()->renderer()->skies()->findClosestSkyByDistance(map_view->getCamera()->position);
	// 	Sky* default_sky = _map_view->getWorld()->renderer()->skies()->findClosestSkyByWeight();
	// 	if (default_sky == nullptr)
	// 		return; // todo error
	// 	else
	// 	{
	// 		loadSelectSky(default_sky);
	// 	}
	// 	});

	connect(_light_tree_filter, &QLineEdit::textChanged, [=](const QString& text)
		{
			if (text.isEmpty())
			{
				// Unhide all items when search text is empty
				for (int i = 0; i < _light_tree->count(); ++i)
				{
					_light_tree->item(i)->setHidden(false);
				}
			}
			else
			{
				for (int i = 0; i < _light_tree->count(); ++i)
				{
					QListWidgetItem* item = _light_tree->item(i);

					bool match = item->text().contains(text, Qt::CaseInsensitive);
					item->setHidden(!match);
				}
			}
		});

	connect(GetSelectedSkyButton, &QPushButton::clicked, [=]() 
		{
			auto const& selected_items = _light_tree->selectedItems();
			if (selected_items.size())
			{
				unsigned int selected_light_id = selected_items.back()->data(Qt::UserRole + 1).toUInt();

				Sky* sky = _map_view->getWorld()->renderer()->skies()->findSkyById(selected_light_id);
				if (sky)
					loadSelectSky(sky);
			}
		});

	connect(_light_tree, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem* item)
		{
			unsigned int selected_light_id = item->data(Qt::UserRole + 1).toUInt();

			Sky* sky = _map_view->getWorld()->renderer()->skies()->findSkyById(selected_light_id);
			if (sky)
				loadSelectSky(sky);
		});


	connect(addNewSkyButton, &QPushButton::clicked, [=]() {

		// get selected sky to duplicate
		Sky* old_sky = nullptr;

		std::string old_name = "";

		// if there is only one light and it is global, we have nothing to duplicate from so allow user to duplicate anything
		// this is REALLY scuffed
		if (!_light_tree->count() || _light_tree->count() == 1)
		{
			if (_light_tree->count() == 1)
			{
				unsigned int old_light_id = _light_tree->item(0)->data(Qt::UserRole + 1).toUInt();

				for (Sky& sky : _map_view->getWorld()->renderer()->skies()->skies)
				{
					if (sky.Id == old_light_id)
					{
						auto test_old_sky = &sky;
						if (test_old_sky != nullptr)
						{
							if (!test_old_sky->global)
							{
								// light isn't global, we have a valid light to duplicate
								old_sky = &sky;
								old_name = _light_tree->item(0)->text().toStdString();
							}
						}
						break;
					}
				}
			}
			if (old_sky == nullptr)
			{
				// TODO input popup, allow copying a light from a dbc row id
			}
		}
		else if (_light_tree->selectedItems().size())
		{
			auto const selected_items = _light_tree->selectedItems();

			old_name = selected_items.back()->text().toStdString();
			unsigned int selected_light_id = selected_items.back()->data(Qt::UserRole + 1).toUInt();

			for (Sky& sky : _map_view->getWorld()->renderer()->skies()->skies)
			{
				if (sky.Id == selected_light_id)
				{
					old_sky = &sky;
					break;
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
			item->setData(Qt::UserRole + 1, QVariant(new_light_id));

			item->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::sun));

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
					// 
					// _map_view->_camera.position = _curr_sky->pos;
					// _map_view->_camera.position.z += 100;
					// get terrain's height for Z axis.
					// auto chunk = _world->getChunkAt(glm::vec3(_curr_sky->pos.x, _curr_sky->pos.y, _curr_sky->pos.z)); // need to load tile first
					// if (chunk != nullptr)
					// _map_view->_camera.position.z = chunk->getMaxHeight() + 20.0f; 
					// TODO : initialize

		});

	connect(param_combobox, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
		
		Sky* curr_sky = get_selected_sky();

		if (!curr_sky)
		{
			assert(false);
			return;
		}
		if (!curr_sky->skyParams[index])
			return;


		curr_sky->curr_sky_param = index;
		load_light_param(index);

		// update rendering to selected param
		updateLightning();

		});

	connect(save_current_sky_button, &QPushButton::clicked, [=]() {
		Sky* curr_sky = get_selected_sky();
		if (!curr_sky)
		{
			assert(false);
			return;
		}
		curr_sky->save_to_dbc();
		});

	connect(pos_x_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		get_selected_sky()->pos.x = v;
		updateLightning();

		});

	connect(pos_y_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		get_selected_sky()->pos.z = v;
		updateLightning();
		});

	connect(pos_z_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		get_selected_sky()->pos.y = v;
		updateLightning();
		});

	connect(inner_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		get_selected_sky()->r1 = v;
		updateLightning();
		});

	connect(outer_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
		get_selected_sky()->r2 = v;
		updateLightning();
		});

	connect(glow_slider, &QSlider::valueChanged, [&](int v) {
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->set_glow(v / 100);
		updateLightning();
		});
	
	connect(highlight_sky_checkbox, &QCheckBox::stateChanged, [&](int state) {
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->set_highlight_sky(state);
		updateLightning();
		});

	connect(shallow_water_alpha_slider, &QSlider::valueChanged, [&](int v) {
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->set_river_shallow_alpha(v / 100);
		updateLightning();
		});

	connect(deep_water_alpha_slider, &QSlider::valueChanged, [&](int v) {
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->set_river_deep_alpha(v / 100);
		updateLightning();
		});

	connect(shallow_ocean_alpha_slider, &QSlider::valueChanged, [&](int v) {
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->set_ocean_shallow_alpha(v / 100);
		updateLightning();
		});

	connect(deep_ocean_alpha_slider, &QSlider::valueChanged, [&](int v) {
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->set_ocean_deep_alpha(v / 100);
		updateLightning();
		});

	// connect(skybox_model_lineedit, &QLineEdit::textChanged, [&](std::string v) {
	QLineEdit::connect(skybox_model_lineedit, &QLineEdit::textChanged
	, [=]
	{
	auto text = skybox_model_lineedit->text().toStdString();
	if (text.empty())
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->skybox.reset();
	else
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->skybox.emplace(text.c_str(), _world->getRenderContext());
	
	updateLightning();
	});

	connect(skybox_flag_1, &QCheckBox::stateChanged, [&](int state) {
	if (state) // set bit
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->skyboxFlags |= (1 << (0));
	else // remove bit
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->skyboxFlags &= ~(1 << (0));
	});

	connect(skybox_flag_2, &QCheckBox::stateChanged, [&](int state) {
	if (state) // set bit
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->skyboxFlags |= (1 << (1));
	else // remove bit
		get_selected_sky()->getParam(param_combobox->currentIndex()).value()->skyboxFlags &= ~(1 << (1));
	});

	// connect(floats_editor_button, &QPushButton::clicked, [=]() {
	// LightFloatsEditor * Editor = new LightFloatsEditor(_map_view, _curr_sky->skyParams[param_combobox->currentIndex()], this);
	// 
	// 	Editor->show();
	// );

}

void LightEditor::loadSelectSky(Sky* _curr_sky)
{
	assert(_curr_sky->getId());
	if (!_curr_sky->getId())
		return;

	_selected_sky_id = _curr_sky->getId();
	// Sky* curr_sky = get_selected_sky();

	QSignalBlocker const _1(pos_x_spin);
	QSignalBlocker const _2(pos_y_spin);
	QSignalBlocker const _3(pos_z_spin);
	QSignalBlocker const _4(inner_radius_spin);
	QSignalBlocker const _5(outer_radius_spin);


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
	std::string light_name = getLightName(_curr_sky->Id, _curr_sky->global, _curr_sky->zone_light);

	std::stringstream ss;
	ss << _curr_sky->Id << "-" << light_name;
	if (_curr_sky->global && _map_view->getWorld()->renderer()->skies()->using_fallback_global)
		ss << " (Fallback)";
	lightid_label->setText(QString::fromStdString(ss.str().c_str()));

	// name_line_edit->setText(QString::fromStdString(_curr_sky->name));
	name_line_edit->setText(QString::fromStdString(light_name));

	global_light_chk->setChecked(_curr_sky->global);
	pos_x_spin->setEnabled(!_curr_sky->global);
	pos_y_spin->setEnabled(!_curr_sky->global);
	pos_z_spin->setEnabled(!_curr_sky->global);
	inner_radius_spin->setEnabled(!_curr_sky->global);
	outer_radius_spin->setEnabled(!_curr_sky->global);

	zone_light_chk->setChecked(_curr_sky->zone_light);

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

void LightEditor::UpdateToolTime()
{
	QSignalBlocker const blocker(_lightning_info_dialog->_time_dial);
	_lightning_info_dialog->_time_dial->setValue(_map_view->getWorld()->time);

	QSignalBlocker TimeHourBlocker(_lightning_info_dialog->TimeSelectorHour);
	QSignalBlocker TimeminutesBlocker(_lightning_info_dialog->TimeSelectorMin);

	int ConvertedTime = _map_view->getWorld()->time * (24 * 60) / MAX_TIME_VALUE;

	int Hour = floor(ConvertedTime / 60);
	int Min = ConvertedTime % 60;

	_lightning_info_dialog->TimeSelectorHour->setValue(Hour);
	_lightning_info_dialog->TimeSelectorMin->setValue(Min);

	UpdateWorldTime();
}

void LightEditor::updateActiveLights()
{
	_active_lights_tree->clear();
	
	Sky* global_sky = nullptr;

	// if we have one light with 1.0 global is entirely replaced, otherwise they are blended with global.
	bool use_global = true;

	int active_count = 0;
	for (auto& sky : _map_view->getWorld()->renderer()->skies()->skies)
	{
		if (sky.global)
			global_sky = &sky;

		// global always has weight 0.0f
		if (sky.weight > 0.f)
		{
			if (sky.weight == 1.0f)
				use_global = false;

			// create item, copypasta from the other tree widget
			QListWidgetItem* item = new QListWidgetItem();

			std::stringstream ss;
			unsigned int light_id = sky.getId();
			item->setData(Qt::UserRole + 1, QVariant(light_id));

			std::string const light_name = getLightName(light_id, false, sky.zone_light);
			// std::string const sky_percent = std::to_string(sky.weight * 100.0f) + "% :";

			ss << "[" << std::fixed << std::setprecision(1) << (sky.weight * 100.0f);
			ss << "%] " << light_id << "-" << light_name;
			item->setText(QString(ss.str().c_str()));
			_active_lights_tree->addItem(item);
			active_count++;
		}
	}

	if (use_global && global_sky)
	{
		QListWidgetItem* item = new QListWidgetItem();

		std::stringstream ss;
		unsigned int light_id = global_sky->getId();
		item->setData(Qt::UserRole + 1, QVariant(light_id));

		std::string const light_name = getLightName(light_id, true);

		ss << "[Global] " << light_id << "-" << light_name;
		item->setText(QString(ss.str().c_str()));
		_active_lights_tree->addItem(item);

		// item->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::sun));
	}

}

void Noggit::Ui::Tools::LightEditor::updateLightningInfo()
{
	Skies* const skies = _map_view->getWorld()->renderer()->skies().get();

	Sky* const highest_weight_sky = _map_view->getWorld()->renderer()->skies()->findClosestSkyByWeight();

	std::string light_name = getLightName(highest_weight_sky->Id, highest_weight_sky->global, highest_weight_sky->zone_light);
	std::stringstream ss;
	ss << highest_weight_sky->Id << "-" << light_name;
	_lightning_info_dialog->_highest_weight_sky_label->setText(QString::fromStdString(ss.str().c_str()));

	// color params
	for (int i = 0; i < NUM_SkyColorNames; ++i)
	{
		QLabel* colorIconLabel = _lightning_info_dialog->_current_lightning_colors_labels[i];
		QPixmap colorIcon(16, 16);  // Create a 16x16 px pixmap
		// colorIcon.fill(Qt::transparent);  // Transparent background

		glm::vec3 color = skies->color_set[i];

		QColor customColor = QColor::fromRgbF(color.r, color.g, color.b);

		QPainter painter(&colorIcon);
		painter.setBrush(QBrush(customColor));
		painter.setPen(Qt::NoPen);
		painter.drawRect(0, 0, 16, 16);
		colorIconLabel->setPixmap(colorIcon);
		std::stringstream ss;
		ss << "R:" << (int)(color.r * 255.f);
		ss << "\nG:" << (int)(color.g * 255.f);
		ss << "\nB:" << (int)(color.b * 255.f);
		colorIconLabel->setToolTip(ss.str().c_str());
	}
	// float params
	{
		float fog_distance = skies->fog_distance_end();
		_lightning_info_dialog->_current_lightning_floats_labels[SKY_FOG_DISTANCE]->setText(QString::number(fog_distance, 'f', 2));

		// actual value in storage is multiplier, not start distance
		float fog_multiplier = skies->fog_distance_multiplier();
		float fog_start_distance = skies->fog_distance_start();
		// display format : 16000 (0.1)
		QString formattedString = QString("%1 (%2)").arg(QString::number(fog_multiplier, 'f', 2)).arg(QString::number(fog_start_distance, 'f', 2));
		auto debug_str = formattedString.toStdString();
		_lightning_info_dialog->_current_lightning_floats_labels[SKY_FOG_MULTIPLIER]->setText(formattedString);

		float celestial_glow= skies->celestial_glow();
		_lightning_info_dialog->_current_lightning_floats_labels[SKY_CELESTIAL_GLOW]->setText(QString::number(celestial_glow, 'f', 2));

		float cloud_density = skies->cloud_density();
		_lightning_info_dialog->_current_lightning_floats_labels[SKY_CLOUD_DENSITY]->setText(QString::number(cloud_density, 'f', 2));

		float float_param_unk4 = skies->unknown_float_param4();
		_lightning_info_dialog->_current_lightning_floats_labels[SKY_UNK_FLOAT_PARAM_4]->setText(QString::number(float_param_unk4, 'f', 2));

		float float_param_unk5 = skies->unknown_float_param5();
		_lightning_info_dialog->_current_lightning_floats_labels[SKY_UNK_FLOAT_PARAM_5]->setText(QString::number(float_param_unk5, 'f', 2));
	}
	// light params
	{
		_lightning_info_dialog->_river_shallow_alpha_label_label->setText(QString::number(skies->river_shallow_alpha(), 'f', 2));
		_lightning_info_dialog->_river_deep_alpha_label->setText(QString::number(skies->river_deep_alpha(), 'f', 2));
		_lightning_info_dialog->_ocean_shallow_alpha_label->setText(QString::number(skies->ocean_shallow_alpha(), 'f', 2));
		_lightning_info_dialog->_ocean_deep_alpha_label->setText(QString::number(skies->ocean_deep_alpha(), 'f', 2));
		_lightning_info_dialog->_glow_label->setText(QString::number(skies->glow(), 'f', 2));

		auto param_opt = highest_weight_sky->getCurrentParam();
		if (param_opt.has_value())
		{
			SkyParam* sky_param = param_opt.value();
			bool highlight = sky_param->highlight_sky();
			_lightning_info_dialog->_highlight_label->setText(highlight ? "True" : "False");
		}

	}
}

void LightEditor::updateLightning()
{
	_world->renderer()->skies()->force_update();
	updateLightningInfo();

}

std::string LightEditor::getLightName(int light_id, bool global, bool light_zone)
{
	std::string light_name = "Unnamed Light";
	if (light_names_map.contains(light_id))
	{
		auto new_name = light_name = light_names_map.at(light_id);
		if (!new_name.empty())
		{
			// can try to get from area in the future
			light_name = new_name;
		}
	}
	else if (global)
		light_name = "Global Light";
	else if (light_zone)
		light_name = "Unnamed Zone Light";

	return light_name;
}

void LightEditor::UpdateWorldTime()
{
	if (ActiveEditor.size() == 0)
		return;

	for (int i = 0; i < ActiveEditor.size(); ++i)
		ActiveEditor[i]->UpdateWorldTime();
}

Sky* Noggit::Ui::Tools::LightEditor::get_selected_sky() const
{
	if(!_selected_sky_id)
		return nullptr;

	return _map_view->getWorld()->renderer()->skies()->findSkyById(_selected_sky_id);
}

void LightEditor::load_light_param(int param_id)
{
	Sky* curr_sky = get_selected_sky();

	if (!curr_sky)
	{
		assert(false);
		return;
	}

	auto param_opt = curr_sky->getParam(param_id);
	if (!param_opt.has_value())
 		return;

	SkyParam * sky_param = param_opt.value();

	QSignalBlocker const _1(glow_slider);
	QSignalBlocker const _2(highlight_sky_checkbox);
	QSignalBlocker const _3(shallow_water_alpha_slider);
	QSignalBlocker const _4(deep_water_alpha_slider);
	QSignalBlocker const _5(shallow_ocean_alpha_slider);
	QSignalBlocker const _6(deep_ocean_alpha_slider);

	QSignalBlocker const _7(skybox_model_lineedit);
	QSignalBlocker const _8(skybox_flag_1);
	QSignalBlocker const _9(skybox_flag_2);
	
	int nb_user = 0;
	try
	{
		DBCFile::Record data = gLightDB.getByID(curr_sky->Id);

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

Noggit::Ui::Tools::LightningInfoDialog::LightningInfoDialog(LightEditor* editor, QWidget* parent)
	:_editor(editor)
	, QWidget(parent)
{
	setWindowTitle("Lightning Info");
	setWindowFlags(Qt::Dialog);
	
	auto main_layout = new QHBoxLayout(this);

	auto layout_column1 = new QVBoxLayout(this);
	main_layout->addLayout(layout_column1);
	// layout_column1->setDirection(QBoxLayout::TopToBottom);
	auto layout_column2 = new QVBoxLayout(this);
	main_layout->addLayout(layout_column2);
	// layout_column2->setDirection(QBoxLayout::TopToBottom);
	auto layout_column3 = new QVBoxLayout(this);
	main_layout->addLayout(layout_column3);
	// layout_column3->setDirection(QBoxLayout::TopToBottom);

	// initialize widgets /////

	layout_column1->addWidget(new QLabel("Set current time :", this));
	_time_dial = new QDial(this);
	layout_column1->addWidget(_time_dial);
	_time_dial->setRange(0, DAY_DURATION); // Time Values from 0 to 2880 where each number represents a half minute from midnight to midnight
	_time_dial->setWrapping(true);
	_time_dial->setSliderPosition((int)_editor->_world->time); // to get ingame orientation
	// _time_dial->setInvertedAppearance(true); // sets the position at top
	_time_dial->setToolTip("Time (24hours)");
	_time_dial->setSingleStep(180); // ticks are 360 units (1/8 = 3 hours)

	TimeSelectorHour = new QSpinBox(this);
	TimeSelectorHour->setMinimum(0);
	TimeSelectorHour->setMaximum(23);

	TimeSelectorMin = new QSpinBox(this);
	TimeSelectorMin->setMinimum(0);
	TimeSelectorMin->setMaximum(59);

	auto time_hlayout = new QHBoxLayout(this);
	time_hlayout->addWidget(new QLabel(tr("Hours"), this));
	time_hlayout->addWidget(TimeSelectorHour);
	time_hlayout->addWidget(new QLabel(tr("Minutes"), this));
	time_hlayout->addWidget(TimeSelectorMin);
	time_hlayout->addStretch();
	layout_column1->addLayout(time_hlayout);

	connect(_time_dial, &QDial::valueChanged, [&](int v) // [this]
		{
			_editor->_map_view->getWorld()->time = v;
			_editor->updateLightning();
			_editor->UpdateWorldTime();

			QSignalBlocker TimeHourBlocker(TimeSelectorHour);
			QSignalBlocker TimeminutesBlocker(TimeSelectorMin);

			int ConvertedTime = v * (24 * 60) / MAX_TIME_VALUE;

			int Hour = floor(ConvertedTime / 60);
			int Min = ConvertedTime % 60;

			TimeSelectorHour->setValue(Hour);
			TimeSelectorMin->setValue(Min);
		}
	);

	connect(TimeSelectorHour, &QSpinBox::textChanged, [=](QString)
		{

			int Time = ((TimeSelectorHour->value() * 60) + TimeSelectorMin->value()) * MAX_TIME_VALUE / (23 * 60 + 59);

			_editor->_map_view->getWorld()->time = Time;

			QSignalBlocker TimeHourBlocker(_time_dial);
			_time_dial->setValue(Time);

			_editor->updateLightning();
			_editor->UpdateWorldTime();
		});

	connect(TimeSelectorMin, &QSpinBox::textChanged, [=](QString)
		{
			int Time = ((TimeSelectorHour->value() * 60) + TimeSelectorMin->value()) * MAX_TIME_VALUE / (23 * 60 + 59);

			_editor->_map_view->getWorld()->time = Time;

			QSignalBlocker TimeHourBlocker(_time_dial);
			_time_dial->setValue(Time);

			_editor->updateLightning();
			_editor->UpdateWorldTime();
		});

	// current colors preview
	for (int i = 0; i < NUM_SkyColorNames; ++i)
	{
		std::string color_name = sky_color_names_map.at(i);

		_current_lightning_colors_labels[i] = new QLabel(this);
		QLabel* colorIconLabel = _current_lightning_colors_labels[i];
		QPixmap colorIcon(32, 16);  // Create a 16x16 px pixmap
		// colorIcon.fill(Qt::transparent);  // Transparent background

		// Use QPainter to draw a red square icon
		QPainter painter(&colorIcon);
		painter.setBrush(QBrush(Qt::black));  // Set the brush to red
		painter.setPen(Qt::NoPen);  // No border
		painter.drawRect(0, 0, 32, 16);  // Draw the square
		colorIconLabel->setPixmap(colorIcon);  // Set the pixmap

		// lightningBox_content_layout->addRow(color_name.c_str(), colorIconLabel);
	}
	// current float params preview
	for (int i = 0; i < NUM_SkyFloatParamsNames; ++i)
	{
		_current_lightning_floats_labels[i] = new QLabel("0", this);

		// lightningBox_content_layout->addRow(sky_float_values_names_map.at(i).c_str(), _current_lightning_floats_labels[i]);
	}
	// light params preview
	{
		_river_shallow_alpha_label_label = new QLabel("0", this);
		// lightningBox_content_layout->addRow("Shallow Water Alpha", _river_shallow_alpha_label_label);
		_river_deep_alpha_label = new QLabel("0", this);
		//lightningBox_content_layout->addRow("Deep Water Alpha", _river_deep_alpha_label);
		_ocean_shallow_alpha_label = new QLabel("0", this);
		//lightningBox_content_layout->addRow("Shallow Ocean Alpha", _ocean_shallow_alpha_label);
		_ocean_deep_alpha_label = new QLabel("0", this);
		//lightningBox_content_layout->addRow("Deep Ocean Alpha", _ocean_deep_alpha_label);
		_glow_label = new QLabel("0", this);
		//lightningBox_content_layout->addRow("Glow", _glow_label);
		_highlight_label = new QLabel("0", this);
		//lightningBox_content_layout->addRow("Highlight Sky", _highlight_label);
	}
	///////////////////////////

	layout_column1->addWidget(new QLabel("Highest Weight Light : ", this));
	_highest_weight_sky_label = new QLabel("None/Not initialized", this);
	layout_column1->addWidget(_highest_weight_sky_label);
	// layout_column1->addItem("Highest Weight Light", _highest_weight_sky_label);

	// Light group
	{
		QGroupBox* light_group = new QGroupBox("Light", this);
		auto light_layout = new QFormLayout(light_group);

		int index = LIGHT_GLOBAL_DIFFUSE;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = LIGHT_GLOBAL_AMBIENT;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		light_layout->addRow("Glow", _glow_label);
		light_layout->addRow("Highlight Sky", _highlight_label);

		index = SHADOW_OPACITY;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = SKY_UNK_FLOAT_PARAM_4;
		light_layout->addRow("Unknown_float_param4", _current_lightning_floats_labels[index]);

		index = SKY_UNK_FLOAT_PARAM_5;
		light_layout->addRow("Unknown_float_param5", _current_lightning_floats_labels[index]);

		layout_column1->addWidget(light_group);
	}

	// Fog group
	{
		QGroupBox* fog_group = new QGroupBox("Fog", this);
		auto fog_layout = new QFormLayout(fog_group);

		int index = SKY_FOG_COLOR;
		fog_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		// TODO density

		index = SKY_FOG_DISTANCE;
		fog_layout->addRow("Fog Farclip", _current_lightning_floats_labels[index]);

		index = SKY_FOG_MULTIPLIER;
		fog_layout->addRow("Fog Nearclip", _current_lightning_floats_labels[index]);

		layout_column2->addWidget(fog_group);
	}

	// Sky group
	{
		QGroupBox* light_group = new QGroupBox("Sky", this);
		auto light_layout = new QFormLayout(light_group);

		int index = SKY_COLOR_TOP;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = SKY_COLOR_MIDDLE;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = SKY_COLOR_BAND1;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = SKY_COLOR_BAND2;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = SKY_COLOR_SMOG;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = SUN_COLOR;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = SUN_CLOUD_COLOR;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		layout_column2->addWidget(light_group);
	}

	// Clouds group
	{
		QGroupBox* light_group = new QGroupBox("Clouds", this);
		auto light_layout = new QFormLayout(light_group);

		int index = CLOUD_EMISSIVE_COLOR;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = CLOUD_LAYER1_AMBIENT_COLOR;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		index = CLOUD_LAYER2_AMBIENT_COLOR;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);

		// Cloud type (params.dbc)

		index = SKY_CELESTIAL_GLOW;
		light_layout->addRow(sky_float_values_names_map.at(index).c_str(), _current_lightning_floats_labels[index]);

		index = SKY_CLOUD_DENSITY;
		light_layout->addRow(sky_float_values_names_map.at(index).c_str(), _current_lightning_floats_labels[index]);

		layout_column3->addWidget(light_group);
	}

	// Water group
	{
		QGroupBox* light_group = new QGroupBox("Water", this);
		auto light_layout = new QFormLayout(light_group);

		int index = RIVER_COLOR_LIGHT;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);
		light_layout->addRow("River Close Alpha", _river_shallow_alpha_label_label);

		index = RIVER_COLOR_DARK;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);
		light_layout->addRow("River Far Alpha", _river_deep_alpha_label);

		index = OCEAN_COLOR_LIGHT;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);
		light_layout->addRow("Ocean Close Alpha", _ocean_shallow_alpha_label);


		index = OCEAN_COLOR_DARK;
		light_layout->addRow(sky_color_names_map.at(index).c_str(), _current_lightning_colors_labels[index]);
		light_layout->addRow("Ocean Far Alpha", _ocean_deep_alpha_label);


		layout_column3->addWidget(light_group);
	}

	main_layout->addLayout(layout_column1);
	layout_column1->addStretch();
	main_layout->addLayout(layout_column2);
	layout_column2->addStretch();
	main_layout->addLayout(layout_column3);
	layout_column3->addStretch();

}
