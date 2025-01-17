#include "SoundEntryPickerWindow.h"
#include <noggit/DBC.h>
#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/windows/SoundPlayer/SoundEntryPlayer.h>
// #include <noggit/ui/ZoneIDBrowser.h>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox.h>
#include <QtWidgets/QComboBox.h>
#include <QtWidgets/qlineedit.h>
#include <QSpinBox>
#include <QListWidget>
#include <QToolButton>

#include <sstream>
#include <string>


namespace Noggit
{
    namespace Ui
    {
        SoundEntryPickerWindow::SoundEntryPickerWindow(QPushButton* button, int sound_type_filter, bool allow_none, QWidget* parent)
        : QWidget(parent)
        {
            setWindowTitle("Sound Entry Picker");
            setWindowFlags(Qt::Dialog);

            auto layout = new QHBoxLayout(this);

            auto list_layout = new QVBoxLayout(this);

            auto Editor_layout = new QVBoxLayout(this);

            layout->addLayout(list_layout);
            layout->addLayout(Editor_layout);

            _tree_searchbar = new QLineEdit(this);
            list_layout->addWidget(_tree_searchbar);


            _tree_filter_cbbox = new QComboBox(this);
            _tree_filter_cbbox->addItem("Show All");
            for (auto sound_types : sound_types_names)
            {
                _tree_filter_cbbox->addItem(sound_types.second.c_str());
            }
            list_layout->addWidget(_tree_filter_cbbox);

            _picker_listview = new QListWidget(this);
            _picker_listview->setFixedSize(280, 460);
            _picker_listview->setSelectionMode(QListWidget::SingleSelection);
            _picker_listview->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
            list_layout->addWidget(_picker_listview);

            if (sound_type_filter == -1)
            {
                _tree_filter_cbbox->setCurrentIndex(0);
                filter_sound_type(sound_type_filter, true);
            }
            else
            {
                int row_id = 0;
                for (auto sound_type_pair : sound_types_names)
                {
                    if (sound_type_pair.first == sound_type_filter)
                    {
                        _tree_filter_cbbox->setCurrentIndex(row_id + 1);
                        filter_sound_type(sound_type_filter);
                        break;
                    }
                    row_id++;
                }
            }

            auto select_entry_btn = new QPushButton("Select Entry", this);
            auto select_entry_none_btn = new QPushButton("Select -NONE-", this);
            auto duplicate_entry_btn = new QPushButton("Duplicate selected Entry (create new)", this);
            list_layout->addWidget(duplicate_entry_btn);
            list_layout->addWidget(select_entry_btn);
            list_layout->addWidget(select_entry_none_btn);

            list_layout->addStretch();

            // Editor frame
            QGroupBox* editor_group = new QGroupBox("Edit Selected Entry", this);
            Editor_layout->addWidget(editor_group);

            auto editor_form_layout = new QFormLayout(editor_group);

            _entry_id_lbl = new QLabel(this);
            editor_form_layout->addRow("Id:", _entry_id_lbl);
            _entry_id_lbl->setEnabled(false);

            _name_ledit = new QLineEdit(this);
            editor_form_layout->addRow("Sound Entry Name:", _name_ledit);

            _sound_type_cbbox = new QComboBox(this);
            for (auto sound_type : sound_types_names)
            {
                _sound_type_cbbox->addItem(sound_type.second.c_str(), sound_type.first);
            }
            editor_form_layout->addRow("Sound Type:", _sound_type_cbbox);

            _volume_slider = new QSlider(Qt::Horizontal, this);
            _volume_slider->setMaximum(100);
            // ticks should be 1, should bt QT's default ?
            editor_form_layout->addRow("Volume:", _volume_slider);

            _min_distance_spinbox = new QSpinBox(this);
            _min_distance_spinbox->setMaximum(1000);
            editor_form_layout->addRow("Min Distance:", _min_distance_spinbox);

            _max_distance_spinbox = new QSpinBox(this);
            _max_distance_spinbox->setMaximum(1000);
            editor_form_layout->addRow("Distance Cut off:", _max_distance_spinbox);

            _eax_type_cbbox = new QComboBox(this);
            _eax_type_cbbox->addItem("None");
            _eax_type_cbbox->addItem("Effect 1");
            _eax_type_cbbox->addItem("Effect 2");
            editor_form_layout->addRow("EAX definition:", _eax_type_cbbox);

            _flag6_checkbox = new QCheckBox("Use OS sound settings", this);
            editor_form_layout->addRow(_flag6_checkbox);
            _flag10_checkbox = new QCheckBox("PlaySpellLoopedSound", this);
            editor_form_layout->addRow(_flag10_checkbox);
            _flag11_checkbox = new QCheckBox("SetFrequencyAndVolume", this);
            editor_form_layout->addRow(_flag11_checkbox);

            _directory_ledit = new QLineEdit(this);
            editor_form_layout->addRow("Directory Path", _directory_ledit);

            auto filecount_layout = new QHBoxLayout(this);
            Editor_layout->addLayout(filecount_layout);

            _filescount_lbl = new QLabel(this);
            filecount_layout->addWidget(_filescount_lbl);
            auto add_file_button = new QPushButton("Add sound file");
            filecount_layout->addWidget(add_file_button);

            _files_listview = new QListWidget(this);
            _files_listview->setFixedHeight(400);
            // _files_listview->setFixedSize(280, 460);
            _files_listview->setSelectionMode(QListWidget::SingleSelection);
            Editor_layout->addWidget(_files_listview);


            auto save_music_entry_btn = new QPushButton("Save changes", this);
            Editor_layout->addWidget(save_music_entry_btn, 0, Qt::AlignRight);

            Editor_layout->addStretch();

            /// check if needed
            select_entry(button->property("id").toInt());

            connect(_tree_filter_cbbox, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {

                if (index == 0) // load all
                    filter_sound_type(0, true);
                else
                {
                    int sound_type_id = 0;
                    int row_id = 0;
                    for (auto sound_type : sound_types_names)
                    {
                        if (row_id == index - 1)
                        {
                            sound_type_id = sound_type.first;
                            break;
                        }
                        row_id++;
                    }
                    filter_sound_type(sound_type_id);
                }

                });
            
            connect(_tree_searchbar, &QLineEdit::textChanged, [=](const QString& text)
            {
              if (text.isEmpty())
              {
                // Unhide all items when search text is empty
                for (int i = 0; i < _picker_listview->count(); ++i) {
                  _picker_listview->item(i)->setHidden(false);
                }
              }
              else
              {
                for (int i = 0; i < _picker_listview->count(); ++i) {
                  QListWidgetItem* item = _picker_listview->item(i);

                  bool match = item->text().contains(text, Qt::CaseInsensitive);
                  item->setHidden(!match);
                }
              }
            });

            QObject::connect(_picker_listview, &QListWidget::itemSelectionChanged, [this]()
              {
                QListWidgetItem* const item = _picker_listview->currentItem();
                if (item)
                {
                  select_entry(item->data(Qt::UserRole).toInt());
                }
              }
            );

            connect(select_entry_btn, &QPushButton::clicked, [=]() {
                // auto selection = _picker_listview->selectedItems();
                auto selected_item = _picker_listview->currentItem();
                if (selected_item == nullptr)
                    return;

                button->setProperty("id", selected_item->data(Qt::UserRole).toInt());
                button->setText(selected_item->text());
                this->close();
            });

            connect(select_entry_none_btn, &QPushButton::clicked, [=]() {
                button->setText("-NONE-");
                button->setProperty("id", 0);
                this->close();
            });

            connect(save_music_entry_btn, &QPushButton::clicked, [=]() {
                save_entry(_entry_id);
            });

            connect(add_file_button, &QPushButton::clicked, [=]() {

                int new_row_id = _files_listview->count();
                if (new_row_id > 9)
                    return; // can't have more than 10

                _filenames_ledits[new_row_id] = new QLineEdit(); // set parent in the widget class
                _filenames_ledits[new_row_id]->setText("your_sound_file.mp3");
                auto file_widget = new SoundFileWListWidgetItem(_filenames_ledits[new_row_id], _directory_ledit->text().toStdString());

                auto item = new QListWidgetItem(_files_listview);
                _files_listview->setItemWidget(item, file_widget);
                item->setSizeHint(QSize(_files_listview->width(), _files_listview->height() / 10));

                _files_listview->addItem(item);

                update_files_count();

                });

            connect(duplicate_entry_btn, &QPushButton::clicked, [=]() {
                auto new_id = gSoundEntriesDB.getEmptyRecordID(SoundEntriesDB::ID);

                auto new_record = gSoundEntriesDB.addRecord(new_id);

                _name_ledit->setText("Noggit Unnamed entry");

                save_entry(new_id);

                // add new tree item
                auto item = new QListWidgetItem();
                item->setData(Qt::UserRole, new_id);
                std::stringstream ss;

                _picker_listview->addItem(item);

                select_entry(new_id);

                ss << new_id << "-" << _name_ledit->text().toStdString();
                item->setText(ss.str().c_str());
                });
        }

        void SoundEntryPickerWindow::filter_sound_type(int sound_type, bool load_all)
        {            
            _picker_listview->clear();
            for (DBCFile::Iterator i = gSoundEntriesDB.begin(); i != gSoundEntriesDB.end(); ++i)
            {
                if (!(i->getInt(SoundEntriesDB::SoundType) == sound_type) && !load_all) // it freezes for 2sec if we try to add directly all 13k items
                    continue;

                auto item = new QListWidgetItem();
                item->setData(Qt::UserRole, i->getInt(SoundEntriesDB::ID));

                std::stringstream ss;
                ss << i->getInt(SoundEntriesDB::ID) << "-" << i->getString(SoundEntriesDB::Name);

                if (load_all)
                    ss << "(" << sound_types_names.at(i->getInt(SoundEntriesDB::SoundType)) << ')';
                item->setText(ss.str().c_str());

                _picker_listview->addItem(item);
            }
        }

        void SoundEntryPickerWindow::select_entry(int id)
        {
            if (id != 0)
            {
                // _picker_listview->setCurrentRow(0);
                // doesn't work because of the type filter ! :(
                // _picker_listview->setCurrentRow(gSoundEntriesDB.getRecordRowId(id));
            }
            else
            {
                _picker_listview->setCurrentRow(0);
                return;
            }

            try
            {
                DBCFile::Record record = gSoundEntriesDB.getByID(id);

                _entry_id = id;
                _entry_id_lbl->setText(QString(std::to_string(id).c_str()));

                _name_ledit->setText(record.getString(SoundEntriesDB::Name));

                int sound_type_id = record.getInt(SoundEntriesDB::SoundType);

                int row_id = 0;
                for (auto sound_type : sound_types_names)
                {
                    if (sound_type.first == sound_type_id)
                    {
                        _sound_type_cbbox->setCurrentIndex(row_id);
                        break;
                    }

                    row_id++;
                }

                _eax_type_cbbox->setCurrentIndex(record.getInt(SoundEntriesDB::EAXDef));
                _min_distance_spinbox->setValue(record.getFloat(SoundEntriesDB::minDistance));
                _max_distance_spinbox->setValue(record.getFloat(SoundEntriesDB::distanceCutoff));
                _volume_slider->setValue(record.getFloat(SoundEntriesDB::Volume) * 100);

                int flags = record.getInt(SoundEntriesDB::Flags);

                _flag6_checkbox->setChecked((flags & (1 << (6 - 1))) ? true : false);
                _flag10_checkbox->setChecked((flags & (1 << (10 - 1))) ? true : false);
                _flag11_checkbox->setChecked((flags & (1 << (11 - 1))) ? true : false);
                _flag12 = (flags & (1 << (12 - 1))) ? true : false;

                _directory_ledit->setText(record.getString(SoundEntriesDB::FilePath));

                _files_listview->clear();
                for (int i = 0; i < 10; i++)
                {
                    std::string filename = record.getString(SoundEntriesDB::Filenames + i);
                    // auto freq = record.getInt(SoundEntriesDB::Freq + i);

                    if (filename.empty())
                        continue;

                    _filenames_ledits[i] = new QLineEdit(); // set parent in the widget class
                    _filenames_ledits[i]->setText(filename.c_str());
                    auto file_widget = new SoundFileWListWidgetItem(_filenames_ledits[i], _directory_ledit->text().toStdString());

                    auto item = new QListWidgetItem(_files_listview);
                    _files_listview->setItemWidget(item, file_widget);
                    item->setSizeHint(QSize(_files_listview->width(), _files_listview->height() / 10) );

                    _files_listview->addItem(item);
                }
                update_files_count();

                _sound_advanced_id = record.getInt(SoundEntriesDB::soundEntriesAdvancedID);
            }
            catch (SoundEntriesDB::NotFound)
            {

            }
        }

        void SoundEntryPickerWindow::save_entry(int entry_id)
        {
            try
            {
                DBCFile::Record record = gSoundEntriesDB.getByID(entry_id); 

                record.write(SoundEntriesDB::ID, entry_id);

                int sound_type_id = 0;
                int row_id = 0;
                for (auto sound_type : sound_types_names)
                {
                    if (row_id == _sound_type_cbbox->currentIndex())
                    {
                        sound_type_id = sound_type.first;
                        break;
                    }
                    row_id++;
                }
                record.write(SoundEntriesDB::SoundType, sound_type_id);

                record.writeString(SoundEntriesDB::Name, _name_ledit->text().toStdString());

                // _files_listview->count()
                int i = 0;
                for (;i < _files_listview->count(); i++)
                {
                    record.writeString(SoundEntriesDB::Filenames + i, _filenames_ledits[i]->text().toStdString());
                    record.write(SoundEntriesDB::Freq + i, 1); // TODO. but in 99.9% 1 is fine
                }
                for (;i < 10; i++) // clean up unset entries
                {
                    record.writeString(SoundEntriesDB::Filenames + i, "");
                    record.write(SoundEntriesDB::Freq + i, 0);
                }

                record.writeString(SoundEntriesDB::FilePath, _directory_ledit->text().toStdString());
                record.write(SoundEntriesDB::Volume, _volume_slider->value() / 100.0f); // should be a float

                int flags = 0;
                if (_flag6_checkbox->isChecked())
                    flags  |= (1ULL << (5));
                if (_flag10_checkbox->isChecked())
                    flags |= (1ULL << (9));
                if (_flag11_checkbox->isChecked())
                    flags |= (1ULL << (10));
                if (_flag12)
                    flags |= (1ULL << (11));
                record.write(SoundEntriesDB::Flags, flags);

                record.write(SoundEntriesDB::minDistance, static_cast<float>(_min_distance_spinbox->value()));
                record.write(SoundEntriesDB::distanceCutoff, static_cast<float>(_max_distance_spinbox->value()));
                record.write(SoundEntriesDB::EAXDef, _eax_type_cbbox->currentIndex());
                record.write(SoundEntriesDB::soundEntriesAdvancedID, _sound_advanced_id);

                gSoundEntriesDB.save();
            }
            catch (SoundEntriesDB::NotFound)
            {

            }

        }

        void SoundEntryPickerWindow::update_files_count()
        {
            int file_count = _files_listview->count();
            std::stringstream ss;
            ss << "Files:   " << std::to_string(file_count) << "/10";

            _filescount_lbl->setText(ss.str().c_str());
        }

        SoundFileWListWidgetItem::SoundFileWListWidgetItem(QLineEdit* filename_ledit, std::string dirpath, QWidget* parent) // std::string filename
        : QWidget(parent)
        {
            auto layout = new QHBoxLayout(this);
            layout->addWidget(new QLabel("File:", this));

            // auto lol = _filenames_ledits[i];
            // auto _filename_ledit = new QLineEdit(filename.c_str(), this);
            filename_ledit->setParent(this);
            filename_ledit->setFixedHeight(30);
            layout->addWidget(filename_ledit);

            auto play_sound_button = new QToolButton(this);
            play_sound_button->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::play));
            //play_sound_button->setFixedSize(play_sound_button->size() * 0.5);
            play_sound_button->setFixedSize(20, 20);
            layout->addWidget(play_sound_button);

            // auto removefile_button = new QToolButton(this);
            // removefile_button->setIcon(Noggit::Ui::FontAwesomeIcon(FontAwesome::windowclose));
            //removefile_button->setFixedSize(removefile_button->size() * 0.5);
            // removefile_button->setFixedSize(20, 20);
            // layout->addWidget(removefile_button);


            connect(play_sound_button, &QPushButton::clicked, [=]() {

                if (!filename_ledit->text().toStdString().empty() && !dirpath.empty())
                {
                    auto sound_player = new SoundEntryPlayer(this);
                    sound_player->PlaySingleSoundFile(filename_ledit->text().toStdString(), dirpath);
                    sound_player->show();
                }
                });
        }

}
}
