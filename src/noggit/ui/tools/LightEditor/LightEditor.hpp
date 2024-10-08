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

	static const std::unordered_map <int, std::string> sky_color_names_map = {
		{0	, "Direct"},
		{1	, "Ambiant"},
		{2	, "Sky Top"},
		{3	, "Sky Midle"},
		{4	, "Sky Band 1"},
		{5	, "Sky Band 2"},
		{6	, "Sky Smog"},
		{7	, "Sky Fog"},
		{8	, "Sun"},
		{9	, "Cloud Sun"},
		{10	, "Cloud Emissive"},
		{11	, "Cloud Layer 1 Ambiant"},
		{12	, "Cloud Layer 2 Ambiant"},
		{13	, "Unknown/Unused"},
		{14	, "Ocean Close"},
		{15	, "Ocean Far"},
		{16	, "River Close"},
		{17	, "River Far"}
	};

	static const std::map <int, std::string> sky_float_values_names_map = {
	{0	, "Fog Distance"},
	{1	, "Fog Multiplier"},
	{2	, "Celestial Glow Through"},
	{3	, "Cloud Density"},
	{4	, "Unkown/Unused1"},
	{5	, "Unkown/Unused2"}
	 };

	// TODO : move to definitions files
	static const std::unordered_map <int, std::string> light_names_map = {
		{1	, "Global Light"},
		{2	, "DuskWood"},
		{3	, "Swamp Of Sorrows"},
		{4	, "WestFall n 2"},
		{5	, "Deadwind Pass"},
		{6	, "Blasted Lands"},
		{7	, "Burning Steppes"},
		{8	, "WestFall 2 razan"},
		{9	, "StrangleThorn"},
		{10	, "Badlands  enter"},
		{11	, "WetLands"},
		{12	, "Arathi Highlands"},
		{13	, "Alterac Mountains"},
		{14	, "Swamp Of Sorrows 3"},
		{15	, "Duskwood East al 3"},
		{16	, "Dun Morogh"},
		{17	, "Deadwind Pass Tower"},
		{18	, "Duskwood East2"},
		{19	, "Blasted Lands Demon"},
		{20	, "Burning Steppes2"},
		{21	, "Badlands2"},
		{22	, "Dun Morogh2"},
		{23	, "Dun Morogh3"},
		{24	, "Dun Morogh4"},
		{25	, "Loch Modan"},
		{26	, "Loch Modan 2"},
		{27	, "Tirisfall Glades"},
		{28	, "SilverPine"},
		{29	, "Tirisfall Glades 3"},
		{30	, "PlagueLands03"},
		{31	, "PlagueLandsWest01"},
		{32	, "PlagueLandsWest04"},
		{33	, "PlagueLandsEast02"},
		{34	, "PlagueLandsWestUther"},
		{35	, "PlagueLandsWest05"},
		{36	, "AeriePeak"},
		{37	, "AeriePeakEast"},
		{38	, "AeriePeakWorldTree"},
		{39	, "StrangleBootyBay"},
		{40	, "DuskWoodNagleHouse"},
		{41	, "DuskWood World Tree"},
		{42	, "PlagueLandsEast01"},
		{43	, "PlagueEastCorrupt01"},
		{44	, "Alterac Snow"},
		{45	, "ArathiHighlandsCoast"},
		{46	, "PlagueEastCorrupt02"},
		{47	, "PlagueEastCorrupt03"},
		{48	, "PlagueEastCorrupt04"},
		{49	, "PlagueEastCorrupt05"},
		{50	, "Burning Steppes Fix"},
		{51	, "StormwindIndustrial"},
		{52	, "StormwindIndustrial2"},
		{53	, "Tirisfall Glades 4"},
		{54	, "Outland01"},
		{55	, "HillsbradTowerIsle"},
		{56	, "HillsbradTowerIsDark"},
		{57	, "ZulGurub"},
		{58	, "WetLands2"},
		{59	, "OutlandDemon"},
		{60	, "OutlandDemon2"},
		{61	, "AlteracRavenholt"},
		{62	, "PlagueEastCorrupt06"},
		{63	, "AeriePeakTrollCity"},
		{64	, "Blasted Lands2 ity"},
		{65	, "Blasted Lands3"},
		{66	, "Blasted Lands4"},
		{67	, "Blasterd LandsPortal"},
		{68	, "Swamp Of Sorrows Fix"},
		{69	, "Blasted Lands Peak"},
		{70	, "ProgramerIsleDeadWin"},
		{71	, "Deadwind Swamp"},
		{72	, "SearingGorge01"},
		{73	, "SearingGorge02"},
		{74	, "SearingGorgeFix1"},
		{75	, "SearingGorgeFix2"},
		{76	, "SearingGorgeFix3"},
		{77	, "Stormwind"},
		{78	, "Burning Steppes Fix2"},
		{79	, "PlagueEastElfLodge"},
		{80	, "PlagueEastTroll"},
		{81	, "PlagueLandsEastDarro"},
		{82	, "Global Light"},
		{83	, "Global Light"},
		{84	, "Global Light"},
		{85	, "CavernsSwamp"},
		{86	, "Hillsbrad orrows"},
		{87	, "BlockingLight01"},
		{88	, "BlockingLight02"},
		{89	, "BlockingLight03"},
		{90	, "BlockingLight04"},
		{91	, "BlockingLight05"},
		{92	, "BlockingLight06"},
		{93	, "BlockingLight07"},
		{94	, "BlockingLight08"},
		{95	, "BlockingLight09"},
		{96	, "BlockingLight091"},
		{97	, "Global Light"},
		{98	, "WestFall n 2"},
		{99	, "WestFallDeadmines"},
		{179, "Global Light"},
		{180, "DireMaul"},
		{181, "DirePlague01"},
		{182, "DirePlague02 es"},
		{183, "DirePlague03 zan"},
		{184, "DireDarkshore01"},
		{185, "DireDarkshore03"},
		{186, "DireCorrupt 03"},
		{187, "Ashenvale"},
		{188, "EmeraldDream01"},
		{189, "EmeraldDreamCanyon"},
		{190, "EmeraldTestArea"},
		{191, "Global Light"},
		{192, "Ashenvale"},
		{193, "Barrens ighlands"},
		{194, "Desolace"},
		{195, "Tanaris Desert"},
		{196, "Moonglade"},
		{197, "Darkshore"},
		{198, "Duskwallow ds Demon"},
		{199, "Darkshore2"},
		{200, "Kalidar"},
		{201, "Mullgore"},
		{202, "MullgoreSouth"},
		{203, "DurotarSouth"},
		{204, "DurotarNorth"},
		{205, "BarrensSouth01"},
		{206, "BarrensSouth02"},
		{207, "BarrensSouth03"},
		{208, "Ashenvale02 03"},
		{209, "Ashenvale03"},
		{210, "Ashenvale04"},
		{211, "ThousandNeedles01"},
		{212, "ThousandNeedles02"},
		{213, "ThousandNeedles03"},
		{214, "Felwood 1 sWest05"},
		{215, "Felwood02"},
		{216, "Felwood03 ast"},
		{217, "Feralas01 orldTree"},
		{218, "AshenvaleHellScream"},
		{219, "KalidarDarnassus"},
		{220, "MullgoreBurrow"},
		{221, "BarrensOilRig"},
		{222, "Feralas02"},
		{223, "BarrensDreadMistPeak"},
		{224, "StonetalonBarrens"},
		{225, "StonetalonClearCut"},
		{226, "StonetalonHarpy"},
		{227, "Stonetalon01"},
		{228, "StonetalonPeak"},
		{229, "AshenvaleCoast"},
		{230, "ThousandNeedlesSalt"},
		{231, "DuskwallowBarrens"},
		{232, "FelwoodJadenar"},
		{233, "FelwoodJadenar2"},
		{234, "MullgoreHarpy"},
		{235, "AshenvaleHellScream2"},
		{236, "ThousandNeedles05"},
		{237, "ThousandNeedles06"},
		{238, "Azshara2"},
		{239, "DurotarOgrimmar"},
		{240, "FeralasCoast"},
		{241, "FeralasTwinColossus"},
		{242, "Tanaris Desert2"},
		{243, "Tanaris Desert3"},
		{244, "Tanaries Desert4"},
		{245, "Tanaries Desert5"},
		{246, "Desolace2"},
		{247, "Azshara1 ea2"},
		{248, "UnGoro 1 ea2"},
		{249, "Silithus"},
		{250, "FeralasCoast02"},
		{251, "TanarisCavernsTime"},
		{252, "TanarisOger01"},
		{253, "DuskwallowTheramore"},
		{254, "AshenvaleFelwood"},
		{255, "UnGoroTarPites"},
		{256, "UnGoroRaptorMarsh"},
		{257, "UnGoroTRex"},
		{258, "SilithusSouth"},
		{259, "WinterSpring01"},
		{260, "WinterSpring02"},
		{261, "WinterSpring03"},
		{262, "Hyjal Demon"},
		{263, "Hyjal Demon2"},
		{264, "WinterSpring05"},
		{265, "Hyjal Demon3"},
		{266, "Hyjal Demon4"},
		{267, "FeralasCoast03"},
		{268, "FeralasTwinColossus2"},
		{269, "KalidarBaseRoots"},
		{270, "WinterSpringDemon"},
		{271, "GM Island"},
		{272, "WinterSpringDemon2"},
		{273, "BarrensOasis01"},
		{274, "BarrensOasis02"},
		{275, "BarrensOasis03"},
		{276, "BarrensQuilBoar"},
		{277, "BarrensQuilBoar2"},
		{278, "SilithusUngoroEnter"},
		{279, "UnGoroSilithisEnter"},
		{280, "BarrensSouth04"},
		{281, "BarrensQuilBoar3"},
		{282, "UnGoroRiver01"},
		{283, "UnGoroRiver02"},
		{284, "UnGoroRiver03"},
		{285, "UnGoroRiver04"},
		{286, "UnGoroRiver05"},
		{287, "UnGoroRiver06"},
		{288, "UnGoroRiver07"},
		{289, "Darkshore3"},
		{290, "Felwood03Fix"},
		{291, "FelwoodFix"},
		{292, "DesolaceNightElf1"},
		{293, "DesolaceNightElf2"},
		{294, "AshenvaleIrisLake"},
		{295, "AshenvaleStardstLake"},
		{296, "AshenvaleOrcCamp"},
		{297, "DesolaceFix"},
		{298, "DesolaceFix2"},
		{299, "AshenvaleFurbolg"},
		{300, "AshenvaleDarkshore"},
		{301, "AshenvaleDarkshore2"},
		{302, "WinterSpringDemon3"},
		{303, "WinterSpringWildkin1"},
		{304, "WinterSpringWildkin2"},
		{305, "DesolaceCorrupt1"},
		{306, "DesolaceOrc upt1"},
		{307, "DesolaceCorrupt2"},
		{308, "SilithusNorthWest"},
		{309, "DesolaceNagaIsle"},
		{310, "StonetalonWeb"},
		{311, "StonetalonTauren"},
		{312, "AshenvalePurple01"},
		{313, "AshenvalePurple02"},
		{314, "SilithusFix"},
		{315, "SilithusDemonHamer1"},
		{316, "SilithusDemonHamer2"},
		{317, "SilithusDemonHamer3"}
	};

  class LightEditor : public QWidget
  {
  public:
    LightEditor(MapView* map_view, QWidget* parent = nullptr);

	void UpdateWorldTime();

  private:
    MapView* _map_view;
    World* _world;
    Sky* _curr_sky;
    QPushButton* _color_value_Buttons[18]{ 0 };
    QLabel* _color_value_labels[18]{ 0 };

	std::vector<LightViewPreview*> LightsPreview;
	std::vector<LightViewEditor*> ActiveEditor;

	QWidget* _light_editing_widget;

	QTabWidget* lightning_tabs;
	QPushButton* save_current_sky_button;
	QLabel* lightid_label;

	QCheckBox* global_light_chk;
	QLineEdit* name_line_edit;
	QDoubleSpinBox* pos_x_spin;
	QDoubleSpinBox* pos_y_spin;
	QDoubleSpinBox* pos_z_spin;
	QDoubleSpinBox* inner_radius_spin;
	QDoubleSpinBox* outer_radius_spin;

	QLabel* _nb_param_users;
	QListWidget* _light_tree;
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

    bool _is_new_record = false;

    void load_light_param(int param_id);
	void loadSelectSky(Sky* sky);
  };


}

#endif //NOGGIT_LIGHTEDITOR_HPP




