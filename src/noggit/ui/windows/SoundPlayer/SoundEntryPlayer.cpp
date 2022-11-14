#include "SoundEntryPlayer.h"

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Misc.h>
#include <ClientFile.hpp>
#include <noggit/application/NoggitApplication.hpp>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/QTableView>
#include <QStandardItemModel>
#include <QTableWidgetItem>
#include <QSound>
#include <qtemporaryfile>
#include <QMediaPlayer>
#include <QListWidget>
#include <QToolButton>

#include <iostream>
#include <sstream>
#include <string>


namespace Noggit
{
    namespace Ui
    {
        SoundEntryPlayer::SoundEntryPlayer(QWidget* parent)
            : QWidget(parent)
        {
            setWindowTitle("Sound entry player");
            setWindowFlags(Qt::Tool);

            auto layout = new QVBoxLayout(this);

            // auto sound_id_lbl = new QLabel(this);
            // sound_id_lbl->setText("Sound Entry : " + sound_entry_id);
            // layout->addWidget(sound_id_lbl);

            auto dir_layout = new QHBoxLayout(this);
            layout->addLayout(dir_layout);

            dir_layout->addWidget(new QLabel("Directory:"));
            _directory_lbl = new QLabel(this);
            dir_layout->addWidget(_directory_lbl);

            auto controls_layout = new QHBoxLayout(this);
            layout->addLayout(controls_layout);

            _media_player = new QMediaPlayer(this);

            auto btn_play = new QToolButton(this);
            controls_layout->addWidget(btn_play);
            connect(btn_play, &QToolButton::clicked, _media_player, &QMediaPlayer::play);
            btn_play->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::play));

            auto btn_pause = new QToolButton(this);
            controls_layout->addWidget(btn_pause);
            connect(btn_pause, &QToolButton::clicked, _media_player, &QMediaPlayer::pause);
            btn_pause->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::pause));

            auto btn_stop = new QToolButton(this);
            controls_layout->addWidget(btn_stop);
            connect(btn_stop, &QToolButton::clicked, _media_player, &QMediaPlayer::stop);
            btn_stop->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::stop));

            _position_slider = new QSlider(Qt::Horizontal, this);
            _position_slider->setTickInterval(100);
            _position_slider->setSingleStep(100);
            controls_layout->addWidget(_position_slider);

            _volume_slider = new QSlider(Qt::Horizontal, this);
            _volume_slider->setRange(0, 100);
            _volume_slider->setValue(80);
            _media_player->setVolume(80);


            // controls_layout->addWidget(new QLabel("Volume:"));
            auto btn_volume = new QToolButton(this);
            controls_layout->addWidget(btn_volume);
            // connect(btn_volume, &QToolButton::clicked, _media_player, &QMediaPlayer::stop);
            btn_volume->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::volumeup));
            controls_layout->addWidget(btn_volume);
            controls_layout->addWidget(_volume_slider);

            _files_listview = new QListWidget();
            _files_listview->setSelectionMode(QListWidget::SingleSelection);
            layout->addWidget(_files_listview);

            connect(_files_listview, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) {
                play_selected_sound();
                });

            // connect volume
            connect(_volume_slider, &QSlider::valueChanged, [&](int v) {
                _media_player->setVolume(v);
                });

            // media doesn't start playnig imemdiatly when called, need to use this signal
            connect(_media_player, &QMediaPlayer::durationChanged, this, [&](qint64 dur) {
                _position_slider->setMaximum(dur);
                });

            // connect(_media_player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
            connect(_media_player, &QMediaPlayer::positionChanged, [&](qint64 v) {
                auto testmax = _position_slider->maximum();
            QSignalBlocker const blocker(_position_slider);
            _position_slider->setSliderPosition(v);
                });
            // position bar
            connect(_position_slider, &QSlider::valueChanged, [&](int v) {
                // _media_player->currentMedia().canonicalResource().dataSize();
                _media_player->setPosition(v);
                });
            // _media_player->setPosition();
        }

        void SoundEntryPlayer::LoadSoundsFromSoundEntry(int sound_entry_id)
        {
            if (sound_entry_id == 0)
            {
                this->close();
                this->destroy();
                return;
            }

            // get files list
            DBCFile::Record sound_entry_record = gSoundEntriesDB.getByID(sound_entry_id);

            _directory_lbl->setText(sound_entry_record.getString(SoundEntriesDB::FilePath));

            _volume_slider->setValue(sound_entry_record.getFloat(SoundEntriesDB::Volume) * 100);
            _media_player->setVolume(sound_entry_record.getFloat(SoundEntriesDB::Volume) * 100);

            for (int fileid = 0; fileid < 10; fileid++)
            {
                std::string filename = sound_entry_record.getString(SoundEntriesDB::Filenames + fileid);
                if (!filename.empty())
                {
                    // std::stringstream ss_filepah;
                    // ss_filepath << directory << "\\" << filename;
                    // music_files.push_back(ss_filepah.str());
                    // music_files.push_back(filename);
                    _files_listview->addItem(filename.c_str());
                }
            }
            _files_listview->setCurrentRow(0);
            play_selected_sound();
        }

        void SoundEntryPlayer::PlaySingleSoundFile(std::string filepath, std::string dir_path)
        {
            _directory_lbl->setText(dir_path.c_str());

            _files_listview->clear();
            _files_listview->addItem(filepath.c_str());

            _files_listview->setCurrentRow(0);
            play_selected_sound();
        }

        void SoundEntryPlayer::play_selected_sound()
        {
            std::stringstream filename;
            auto item = _files_listview->selectedItems().back();
            filename << _directory_lbl->text().toStdString() << "\\" << item->text().toStdString();

            if (!Noggit::Application::NoggitApplication::instance()->clientData()->exists(filename.str()))
            {
                LogError << "The requested sound file \"" << filename.str() << "\" does not exist! Oo" << std::endl;
                QMessageBox not_found_messagebox;
                not_found_messagebox.setIcon(QMessageBox::Warning);
                not_found_messagebox.setWindowIcon(QIcon(":/icon"));
                not_found_messagebox.setWindowTitle("File not found");
                std::stringstream ss;
                ss << "The requested sound file \"" << filename.str() << "\" was not found in the client." << std::endl;
                not_found_messagebox.setText(ss.str().c_str());
                not_found_messagebox.exec();
                return;
            }

            BlizzardArchive::ClientFile file(filename.str(), Noggit::Application::NoggitApplication::instance()->clientData());

            auto temp_file = new QTemporaryFile(this); // must parent for the object to be destroyed properly(and file deleted)

            temp_file->open();
            temp_file->write(file.getBuffer(), file.getSize());
            temp_file->close();
            // default tempname is like User\AppData\Local\Temp\Noggit.qrRfsy ...We need to add back the file extension or it won't be read by the player!
            // must rename after closing or it doesn't write correctly
            temp_file->rename(temp_file->fileName() + item->text());
            // file.save(); // saves file to project folder
            // auto save_path = file.getPath().string();

            auto testlol = temp_file->fileName().toStdString();

            _media_player->setMedia(QUrl::fromLocalFile(temp_file->fileName())); // QUrl::fromLocalFile("/Users/me/Music/coolsong.mp3")
            _media_player->play();
        }

        void SoundEntryPlayer::closeEvent(QCloseEvent* event)
        {
            // makes the music stop when closing
            _media_player->stop();
            event->accept();
        }
    }
}
