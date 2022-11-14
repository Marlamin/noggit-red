// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/DBC.h>

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox.h>
#include <QtWidgets/QComboBox.h>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QMediaPlayer>

#include <string>


namespace Noggit
{
    namespace Ui
    {
        class SoundEntryPlayer : public QWidget
        {
            Q_OBJECT
        public:
            SoundEntryPlayer(QWidget* parent = nullptr);
            void LoadSoundsFromSoundEntry(int sound_entry_id);
            void PlaySingleSoundFile(std::string filepath, std::string dir_path);

        private:
            QMediaPlayer* _media_player;

            // QLabel* sound_id_lbl;
            QLabel* _directory_lbl;
            QListWidget* _files_listview;
            QSlider* _volume_slider;
            QSlider* _position_slider;

            void play_selected_sound();

        protected:
            void closeEvent(QCloseEvent* event) override;
        };
    }
}