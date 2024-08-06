#include "ZoneMusicPickerWindow.h"
#include <noggit/ui/windows/SoundPlayer/SoundEntryPlayer.h>
#include "SoundEntryPickerWindow.h"

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
        ZoneMusicPickerWindow::ZoneMusicPickerWindow(QPushButton* button, QWidget* parent)
            : QWidget(parent)
        {
            setWindowTitle("Zone Music Picker");
            setWindowFlags(Qt::Dialog);

            auto layout = new QHBoxLayout(this);

            auto list_layout = new QVBoxLayout(this);

            auto Editor_layout = new QVBoxLayout(this);

            layout->addLayout(list_layout);
            layout->addLayout(Editor_layout);

            _tree_searchbar = new QLineEdit(this);
            list_layout->addWidget(_tree_searchbar);

            _picker_listview = new QListWidget(this);
            _picker_listview->setFixedSize(280, 460);
            _picker_listview->setSelectionMode(QListWidget::SingleSelection);
            list_layout->addWidget(_picker_listview);

            for (DBCFile::Iterator i = gZoneMusicDB.begin(); i != gZoneMusicDB.end(); ++i)
            {
                auto item = new QListWidgetItem();
                item->setData(1, i->getInt(ZoneMusicDB::ID));

                std::stringstream ss;
                ss << i->getInt(ZoneMusicDB::ID) << "-" << i->getString(ZoneMusicDB::Name);
                item->setText(ss.str().c_str());

                // _picker_listview->addItem(ss.str().c_str());
                _picker_listview->addItem(item);
            }

            auto select_entry_btn = new QPushButton("Select Entry", this);
            select_entry_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::check));
            auto select_entry_none_btn = new QPushButton("Select -NONE-", this);
            select_entry_none_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::check));
            auto duplicate_entry_btn = new QPushButton("Duplicate selected Entry (create new)", this);
            duplicate_entry_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
            list_layout->addWidget(duplicate_entry_btn);
            list_layout->addWidget(select_entry_btn);
            list_layout->addWidget(select_entry_none_btn);

            // Editor frame
            QGroupBox* editor_group = new QGroupBox("Edit Selected Entry", this);
            Editor_layout->addWidget(editor_group);

            auto editor_form_layout = new QFormLayout(editor_group);

            _entry_id_lbl = new QLabel(this);
            editor_form_layout->addRow("Id:", _entry_id_lbl);
            _entry_id_lbl->setEnabled(false);

            _name_ledit = new QLineEdit(this);
            editor_form_layout->addRow("Music Set Name:", _name_ledit);


            _day_music_button = new QPushButton("-NONE-", this);
            _day_music_button->setProperty("id", 0);
            connect(_day_music_button, &QPushButton::clicked, [=]() {
                auto window = new SoundEntryPickerWindow(_day_music_button, SoundEntryTypes::ZONE_MUSIC_FILES, true, this);
            window->show();
                });

            auto day_music_layout = new QHBoxLayout(this);
            auto play_day_music_button = new QToolButton(this);
            play_day_music_button->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::play));
            day_music_layout->addWidget(new QLabel("Day Music:"));
            day_music_layout->addWidget(_day_music_button);
            day_music_layout->addWidget(play_day_music_button);
            editor_form_layout->addRow(day_music_layout);

            _night_music_button = new QPushButton("-NONE-", this);
            _night_music_button->setProperty("id", 0);
            connect(_night_music_button, &QPushButton::clicked, [=]() {
                auto window = new SoundEntryPickerWindow(_night_music_button, SoundEntryTypes::ZONE_MUSIC_FILES, true, this);
            window->show();
                });

            auto night_music_layout = new QHBoxLayout(this);
            auto play_night_music_button = new QToolButton(this);
            play_night_music_button->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::play));
            night_music_layout->addWidget(new QLabel("Night Music:"));
            night_music_layout->addWidget(_night_music_button);
            night_music_layout->addWidget(play_night_music_button);
            editor_form_layout->addRow(night_music_layout);
            // editor_form_layout->addRow("Night Music:", _night_music_button);

            QGroupBox* silence_intervals_group = new QGroupBox("Silence Intervals", this);
            Editor_layout->addWidget(silence_intervals_group);
            auto silence_interval_layout = new QVBoxLayout(silence_intervals_group);

            silence_interval_layout->addWidget(new QLabel("Day:"));
            auto day_silence_intervals_layout = new QHBoxLayout(this);
            silence_interval_layout->addLayout(day_silence_intervals_layout);
            day_silence_intervals_layout->addWidget(new QLabel("Min:"));
            _day_min_interval_spinbox = new QSpinBox(this);
            _day_min_interval_spinbox->setMaximum(600000);
            day_silence_intervals_layout->addWidget(_day_min_interval_spinbox);
            day_silence_intervals_layout->addWidget(new QLabel("Max:"));
            _day_max_interval_spinbox = new QSpinBox(this);
            _day_max_interval_spinbox->setMaximum(600000);
            day_silence_intervals_layout->addWidget(_day_max_interval_spinbox);

            silence_interval_layout->addWidget(new QLabel("Night:"));
            auto night_silence_intervals_layout = new QHBoxLayout(this);
            silence_interval_layout->addLayout(night_silence_intervals_layout);
            night_silence_intervals_layout->addWidget(new QLabel("Min:"));
            _night_min_interval_spinbox = new QSpinBox(this);
            _night_min_interval_spinbox->setMaximum(600000);
            night_silence_intervals_layout->addWidget(_night_min_interval_spinbox);
            night_silence_intervals_layout->addWidget(new QLabel("Max:"));
            _night_max_interval_spinbox = new QSpinBox(this);
            _night_max_interval_spinbox->setMaximum(600000);
            night_silence_intervals_layout->addWidget(_night_max_interval_spinbox);

            auto save_music_entry_btn = new QPushButton("Save changes", this);
            Editor_layout->addWidget(save_music_entry_btn, 0, Qt::AlignRight);

            Editor_layout->addStretch();

            /// check if needed
            select_entry(button->property("id").toInt());

            connect(_tree_searchbar, &QLineEdit::textChanged, [=](QString obj) {
                if (obj.isEmpty())
                {
                    // unhide all
                }

            // hide all items
            for (int i = 0; i < _picker_listview->count(); i++)
            {
                auto item = _picker_listview->item(i);
                item->setHidden(true);
            }
            // unhide matching items
            auto matching_items = _picker_listview->findItems(obj, Qt::MatchContains);

            for (auto item : matching_items)
            {
                item->setHidden(false);
            }
                });

            connect(_picker_listview, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) {
                select_entry(item->data(1).toInt());
                });

            connect(select_entry_btn, &QPushButton::clicked, [=]() {
                // auto selection = _picker_listview->selectedItems();
                auto selected_item = _picker_listview->currentItem();
            if (selected_item == nullptr)
                return;

            button->setProperty("id", selected_item->data(1).toInt());
            button->setText(selected_item->text());
            this->close();
                });

            connect(select_entry_none_btn, &QPushButton::clicked, [=]() {
                button->setText("-NONE-");
            button->setProperty("id", 0);
            this->close();
                });

            connect(play_day_music_button, &QPushButton::clicked, [=]() {
                auto sound_entry = _day_music_button->property("id").toInt();
            if (sound_entry)
            {
                auto sound_player = new SoundEntryPlayer(this);
                sound_player->LoadSoundsFromSoundEntry(sound_entry);
                sound_player->show();
            }
                });

            connect(play_night_music_button, &QPushButton::clicked, [=]() {
                auto sound_entry = _night_music_button->property("id").toInt();
            if (sound_entry)
            {
                auto sound_player = new SoundEntryPlayer(this);
                sound_player->LoadSoundsFromSoundEntry(sound_entry);
                sound_player->show();
            }
                });

            connect(save_music_entry_btn, &QPushButton::clicked, [=]() {
                save_entry(_entry_id);
                });

            connect(duplicate_entry_btn, &QPushButton::clicked, [=]() {
                auto new_id = gZoneMusicDB.getEmptyRecordID(ZoneMusicDB::ID);

            auto new_record = gZoneMusicDB.addRecord(new_id);

            _name_ledit->setText("Noggit Unnamed entry");

            save_entry(new_id);

            // add new tree item
            auto item = new QListWidgetItem();
            item->setData(1, new_id);
            std::stringstream ss;

            _picker_listview->addItem(item);

            select_entry(new_id);

            ss << new_id << "-" << _name_ledit->text().toStdString();
            item->setText(ss.str().c_str());
                });


        }
        void ZoneMusicPickerWindow::select_entry(int id)
        {
            if (id)
            {
                try
                {
                    _picker_listview->setCurrentRow(gZoneMusicDB.getRecordRowId(id));
                }
                catch (ZoneMusicDB::NotFound)
                {

                }
            }
            else
            {
                _picker_listview->setCurrentRow(0);
                return;
            }

            try
            {
                DBCFile::Record record = gZoneMusicDB.getByID(id);

                _entry_id = id;
                _entry_id_lbl->setText(QString(std::to_string(id).c_str()));

                _name_ledit->setText(record.getString(ZoneMusicDB::Name));

                int day_sound_entry = record.getInt(ZoneMusicDB::DayMusic);
                int night_sound_entry = record.getInt(ZoneMusicDB::NightMusic);

                if (day_sound_entry != 0 && gSoundEntriesDB.CheckIfIdExists(day_sound_entry)) // some entries reference sound entries that don't exist
                {
                    DBCFile::Record day_sound_record = gSoundEntriesDB.getByID(day_sound_entry);
                    std::stringstream ss_day;
                    ss_day << day_sound_entry << "-" << day_sound_record.getString(SoundEntriesDB::Name);
                    _day_music_button->setText(ss_day.str().c_str());
                    _day_music_button->setProperty("id", day_sound_entry);
                }
                else
                {
                    _day_music_button->setText("-NONE-");
                    _day_music_button->setProperty("id", 0);
                }

                if (night_sound_entry != 0 && gSoundEntriesDB.CheckIfIdExists(night_sound_entry))
                {
                    DBCFile::Record night_sound_record = gSoundEntriesDB.getByID(night_sound_entry);
                    std::stringstream ss_night;
                    ss_night << night_sound_entry << "-" << night_sound_record.getString(SoundEntriesDB::Name);
                    _night_music_button->setText(ss_night.str().c_str());
                    _night_music_button->setProperty("id", night_sound_entry);
                }
                else
                {
                    _night_music_button->setText("-NONE-");
                    _night_music_button->setProperty("id", 0);
                }

                _day_min_interval_spinbox->setValue(record.getInt(ZoneMusicDB::SilenceIntervalMinDay));
                _day_max_interval_spinbox->setValue(record.getInt(ZoneMusicDB::SilenceIntervalMaxDay));
                _night_min_interval_spinbox->setValue(record.getInt(ZoneMusicDB::SilenceIntervalMinNight));
                _night_max_interval_spinbox->setValue(record.getInt(ZoneMusicDB::SilenceIntervalMaxNight));
            }
            catch (ZoneMusicDB::NotFound)
            {

            }
        }

        void ZoneMusicPickerWindow::save_entry(int entry_id)
        {
            try
            {
                DBCFile::Record record = gZoneMusicDB.getByID(entry_id); // is_new_record ? gLightDB.addRecord(Id) : gLightDB.getByID(Id);


                record.write(ZoneMusicDB::ID, entry_id);
                record.writeString(ZoneMusicDB::Name, _name_ledit->text().toStdString());
                record.write(ZoneMusicDB::SilenceIntervalMinDay, _day_min_interval_spinbox->value());
                record.write(ZoneMusicDB::SilenceIntervalMaxDay, _day_max_interval_spinbox->value());
                record.write(ZoneMusicDB::SilenceIntervalMinNight, _night_min_interval_spinbox->value());
                record.write(ZoneMusicDB::SilenceIntervalMaxNight, _night_max_interval_spinbox->value());
                record.write(ZoneMusicDB::DayMusic, _day_music_button->property("id").toInt());
                record.write(ZoneMusicDB::NightMusic, _night_music_button->property("id").toInt());

                gZoneMusicDB.save();
            }
            catch (ZoneMusicDB::NotFound)
            {

            }
        }
    }
}
