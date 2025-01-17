// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>

#include <string>

class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QListWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QSpinBox;

namespace Noggit
{
    namespace Ui
    {
        // struct SoundEntryFilename
        // {
        //     SoundEntryFilename(std::string f, int freq);
        // 
        //     std::string filename;
        //     int freq;
        // };
        // };

        class SoundFileWListWidgetItem : public QWidget
        {
            Q_OBJECT
        public:
            SoundFileWListWidgetItem(QLineEdit* filename_ledit, std::string dirpath, QWidget* parent = nullptr);
            // QLineEdit* _filename_ledit;
        private:

        };

        class SoundEntryPickerWindow : public QWidget
        {
            Q_OBJECT
        public:
            SoundEntryPickerWindow(QPushButton* button, int sound_type_filter = -1, bool allow_none = true, QWidget* parent = nullptr);

        private:
            QComboBox* _tree_filter_cbbox;
            QLineEdit* _tree_searchbar;
            QListWidget* _picker_listview;

            int _entry_id = 0;
            QLabel* _entry_id_lbl;
            QLineEdit* _name_ledit;

            QComboBox* _sound_type_cbbox;
            QComboBox* _eax_type_cbbox;
            QSpinBox* _min_distance_spinbox;
            QSpinBox* _max_distance_spinbox;
            QSlider* _volume_slider;

            QCheckBox* _flag6_checkbox;
            QCheckBox* _flag10_checkbox;
            QCheckBox* _flag11_checkbox;

            QLineEdit* _directory_ledit;
            QLabel* _filescount_lbl;
            QListWidget* _files_listview;
            QLineEdit* _filenames_ledits[10]{ 0 };
            QSpinBox* _freqs_spinboxes[10]{ 0 };

            bool _flag12; // 7 users and unknown definition, not adding it to the UI.

            // TODO sound advanced. shouldn't use a picker as it is unique for each sound, just add an optional advanced layout without maybe a checkbox. maybe it is in a separate tab ?
            int _sound_advanced_id = 0;

            void filter_sound_type(int sound_type, bool load_all = false);
            void select_entry(int id);
            void save_entry(int entry_id);
            void update_files_count();
            // void duplicate_entry();
        };

        enum SoundEntryTypes
        {
            SPELLS = 1,
            UI,
            FOOTSTEPS,
            COMBAT_IMPACTS,
            COMBAT_SWINGS = 6,
            GREETINGS, 
            CASTING,
            ITEM_USE_SOUNDS,
            MONSTER_SOUNDS,
            VOCAL_UI_SOUNDS = 12,
            POINT_SOUND_EMITTERS,
            DOODAD_SOUNDS,
            DEATH = 16,
            NPC_SOUNDS,
            FOLEY_SOUNDS = 19,
            FOOTSTEPS_SPLASHES,
            CHARACTER_SPLASH_SOUNDS,
            WATERVOLUME_SOUNDS,
            TRADESKILL,
            TERRAIN_EMITER,
            GAME_OBJECTS,
            SPELL_FIZZLES,
            CREATURE_LOOPS,
            ZONE_MUSIC_FILES,
            EMOTES,
            CINEMATIC_MUSIC,
            CINEMATIC_VOICE,
            ZONE_AMBIENCE = 50,
            SOUND_EMITTERS = 52,
            VEHICLE_STATES
        };

        static std::map <int, std::string> sound_types_names = {
            {SPELLS	, "Spells"},
            {UI	, "UI"},
            {FOOTSTEPS	, "Footsteps"},
            {COMBAT_IMPACTS	, "Combat Impacts"},
            {COMBAT_SWINGS	, "Combat Swings"},
            {GREETINGS, "Greetings"},
            {CASTING, "Casting"},
            {ITEM_USE_SOUNDS, "Item Use Sound"},
            {MONSTER_SOUNDS, "Monster Sound"},
            {VOCAL_UI_SOUNDS, "VocalUISound"},
            {POINT_SOUND_EMITTERS, "Point Sound Emitter"},
            {DOODAD_SOUNDS, "Doodad Sounds"},
            {DEATH, "Death Thud Sounds"},
            {NPC_SOUNDS, "NPC Sounds"},
            {FOLEY_SOUNDS, "Foley Sound"},
            {FOOTSTEPS_SPLASHES, "Footsteps(Splashes)"},
            {CHARACTER_SPLASH_SOUNDS, "CharacterSplashSounds"},
            {WATERVOLUME_SOUNDS, "WaterVolume Sounds"},
            {TRADESKILL, "Tradeskill Sounds"},
            {TERRAIN_EMITER, "Terrain Emitter Sounds"},
            {GAME_OBJECTS, "Game Object Sounds"},
            {SPELL_FIZZLES, "SpellFizzles"},
            {CREATURE_LOOPS, "CreatureLoops"},
            {ZONE_MUSIC_FILES, "Zone Music Files"},
            {EMOTES, "Character Macro Line"},
            {CINEMATIC_MUSIC, "Cinematic Music"},
            {CINEMATIC_VOICE, "Cinematic Voice"},
            {ZONE_AMBIENCE, "Zone Ambience"},
            {SOUND_EMITTERS, "Sound Emitters"},
            {VEHICLE_STATES, "Vehicle States"}
        };
    }
}

