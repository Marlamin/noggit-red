// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LIGHTEDITOR_HPP
#define NOGGIT_LIGHTEDITOR_HPP

#include <QWidget>
#include <noggit/MapView.h>
#include <QtWidgets/qtreewidget.h>
#include <noggit/ui/widgets/LightViewWidget.h>
#include <QTabWidget>

namespace Noggit::Ui::Tools
{

	static const std::map <int, std::string> sky_color_names_map = {
		{0	, "Light Direct (diffuse)"},
		{1	, "Light Ambiant - Sky"},
		{2	, "Sky Top"},
		{3	, "Sky Midle"},
		{4	, "Sky Band 1"},
		{5	, "Sky Band 2"},
		{6	, "Sky Smog"},
		{7	, "Base Fog"},
		{8	, "Shadow Opacity(Unused)"}, //Unknown/unused in 3.3.5. in new format this row was remvoed and moved to index 17 (after river far)
		{9	, "Sun"},
		{10	, "Cloud Sun"},
		{11	, "Cloud Emissive"},
		{12	, "Cloud Layer 1 Ambiant"},
		{13	, "Cloud Layer 2 Ambiant"}, // Unknown / unused in 3.3.5 ? This value was ported to Cloud Layer 2 Ambient Color in the new format
		{14	, "Ocean Shallow"},
		{15	, "Ocean Deep"},
		{16	, "River Shallow"},
		{17	, "River Deep"}
	};

	static const std::map <int, std::string> sky_float_values_names_map = {
	{0	, "Fog Distance"},
	{1	, "Fog Multiplier"},
	{2	, "Celestial Glow Through"},
	{3	, "Cloud Density"},
	{4	, "Unkown/Unused1"},
	{5	, "Unkown/Unused2"}
	 };

	// last official light.dbc id naming from .lit client files
	int constexpr blizzlikeNameDefinitionsEnd = 418;
	// last blizzzard light.dbc id
	int constexpr blizzlikeSkiesEndWrath = 2538;

	static std::unordered_map <int, std::string> light_names_map;

	class LightEditor;

	class LightningInfoDialog : public QWidget
	{
		Q_OBJECT
	public:
		LightningInfoDialog(LightEditor* editor, QWidget* parent = nullptr);

	// private:
		// current lightning info preview widgets
		LightEditor* _editor;

		QDial* _time_dial;
		QSpinBox* TimeSelectorHour;
		QSpinBox* TimeSelectorMin;

		QLabel* _highest_weight_sky_label;
		QLabel* _current_lightning_colors_labels[18]{ 0 };
		QLabel* _current_lightning_floats_labels[6]{ 0 };

		QLabel* _river_shallow_alpha_label_label;
		QLabel* _river_deep_alpha_label;
		QLabel* _ocean_shallow_alpha_label;
		QLabel* _ocean_deep_alpha_label;
		QLabel* _glow_label;
		QLabel* _highlight_label;
	};

  class LightEditor : public QWidget
  {
  public:
    LightEditor(MapView* map_view, QWidget* parent = nullptr);

		void UpdateToolTime(); // update on time change
		void updateActiveLights(); // only need to update on position change
		void updateLightningInfo(); // needs to be updated on time change AND position change

		void UpdateWorldTime();
		void updateLightning();

    World* _world;
    MapView* _map_view;

		LightningInfoDialog* _lightning_info_dialog;

  private:

		std::string getLightName(int light_id, bool global, bool light_zone = false);

		int _selected_sky_id = 0;
		Sky* get_selected_sky() const;

		std::vector<LightViewPreview*> LightsPreview;
		std::vector<LightViewEditor*> ActiveEditor;

		QWidget* _light_editing_widget;
		QTabWidget* lightning_tabs;

		// QDial* _time_dial;
		QListWidget* _active_lights_tree;

		QLineEdit* _light_tree_filter;
		QListWidget* _light_tree;

		QPushButton* save_current_sky_button;
		QLabel* lightid_label;

		QCheckBox* global_light_chk;
		QCheckBox* zone_light_chk;
		QLineEdit* name_line_edit;
		QDoubleSpinBox* pos_x_spin;
		QDoubleSpinBox* pos_y_spin;
		QDoubleSpinBox* pos_z_spin;
		QDoubleSpinBox* inner_radius_spin;
		QDoubleSpinBox* outer_radius_spin;

		QLabel* _nb_param_users;
    QComboBox* param_combobox;
    QSlider* glow_slider;
    QCheckBox* highlight_sky_checkbox;

    QLineEdit* skybox_model_lineedit;
		QCheckBox* skybox_flag_1;
		QCheckBox* skybox_flag_2;

    QSlider* shallow_water_alpha_slider;
    QSlider* deep_water_alpha_slider;
    QSlider* shallow_ocean_alpha_slider;
    QSlider* deep_ocean_alpha_slider;

    void load_light_param(int param_id);
		void loadSelectSky(Sky* new_sky);
  };


}

#endif //NOGGIT_LIGHTEDITOR_HPP




