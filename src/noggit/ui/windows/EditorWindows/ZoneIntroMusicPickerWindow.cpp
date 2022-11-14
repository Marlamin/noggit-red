#include <noggit/ui/windows/SoundPlayer/SoundEntryPlayer.h>
#include "ZoneIntroMusicPickerWindow.h"
// #include <noggit/ui/ZoneIDBrowser.h>
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
        ZoneIntroMusicPickerWindow::ZoneIntroMusicPickerWindow(QPushButton* button, QWidget* parent)
            : QWidget(parent)
        {
            setWindowTitle("Zone Intro Music Picker");
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

            for (DBCFile::Iterator i = gZoneIntroMusicTableDB.begin(); i != gZoneIntroMusicTableDB.end(); ++i)
            {
                auto item = new QListWidgetItem();
                item->setData(1, i->getInt(ZoneIntroMusicTableDB::ID));

                std::stringstream ss;
                ss << i->getInt(ZoneIntroMusicTableDB::ID) << "-" << i->getString(ZoneIntroMusicTableDB::Name);
                item->setText(ss.str().c_str());

                _picker_listview->addItem(item);
            }

            auto select_entry_btn = new QPushButton("Select Entry", this);
            auto select_entry_none_btn = new QPushButton("Select -NONE-", this);
            auto duplicate_entry_btn = new QPushButton("Duplicate selected Entry (create new)", this);
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
            editor_form_layout->addRow("Intro Music Name:", _name_ledit);


            _sound_button = new QPushButton("-NONE-", this);
            _sound_button->setProperty("id", 0);
            connect(_sound_button, &QPushButton::clicked, [=]() {
                auto window = new SoundEntryPickerWindow(_sound_button, SoundEntryTypes::ZONE_MUSIC_FILES, false, this);
                window->show();
                });
            auto music_layout = new QHBoxLayout(this);
            auto play_music_button = new QToolButton(this);
            play_music_button->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::play));
            music_layout->addWidget(new QLabel("Music sound entry:"));
            music_layout->addWidget(_sound_button);
            music_layout->addWidget(play_music_button);
            editor_form_layout->addRow(music_layout);

            _min_delay_spinbox = new QSpinBox(this);
            _min_delay_spinbox->setMaximum(300);
            editor_form_layout->addRow("Min Delay (minutes):", _min_delay_spinbox);

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

            connect(play_music_button, &QPushButton::clicked, [=]() {
                auto sound_entry = _sound_button->property("id").toInt();
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
                auto new_id = gZoneIntroMusicTableDB.getEmptyRecordID(ZoneIntroMusicTableDB::ID);

                auto new_record = gZoneIntroMusicTableDB.addRecord(new_id);

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

        void ZoneIntroMusicPickerWindow::select_entry(int id)
        {
            _entry_id = id;

            if (id != 0)
                _picker_listview->setCurrentRow(gZoneIntroMusicTableDB.getRecordRowId(id));
            else
            {
                _picker_listview->setCurrentRow(0);
                return;
            }

            _entry_id_lbl->setText(QString(std::to_string(id).c_str()));

            DBCFile::Record record = gZoneIntroMusicTableDB.getByID(id);

            _name_ledit->setText(record.getString(ZoneIntroMusicTableDB::Name));

            int sound_entry = record.getInt(ZoneIntroMusicTableDB::SoundId);

            if (sound_entry != 0 && gSoundEntriesDB.CheckIfIdExists(sound_entry))
            {
                DBCFile::Record sound_record = gSoundEntriesDB.getByID(sound_entry);
                std::stringstream ss;
                ss << sound_entry << "-" << sound_record.getString(SoundEntriesDB::Name);
                _sound_button->setText(ss.str().c_str());
                _sound_button->setProperty("id", sound_entry);
            }
            else
            {
                _sound_button->setText("-NONE-");
                _sound_button->setProperty("id", 0);
            }

            _priority = record.getInt(ZoneIntroMusicTableDB::Priority); // always 1 except for 1 test entry

            _min_delay_spinbox->setValue(record.getInt(ZoneIntroMusicTableDB::MinDelayMinutes));
        }

        void ZoneIntroMusicPickerWindow::save_entry(int entry_id)
        {
            DBCFile::Record record = gZoneIntroMusicTableDB.getByID(entry_id); // is_new_record ? gLightDB.addRecord(Id) : gLightDB.getByID(Id);


            record.write(ZoneIntroMusicTableDB::ID, entry_id);
            record.writeString(ZoneIntroMusicTableDB::Name, _name_ledit->text().toStdString());
            record.write(ZoneIntroMusicTableDB::SoundId, _sound_button->property("id").toInt());
            record.write(ZoneIntroMusicTableDB::Priority, _priority);
            record.write(ZoneIntroMusicTableDB::MinDelayMinutes, _min_delay_spinbox->value());

            gZoneIntroMusicTableDB.save();
        }
    }
}
