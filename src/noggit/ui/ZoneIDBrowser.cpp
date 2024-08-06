// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/ZoneIDBrowser.h>
#include <noggit/ui/windows/SoundPlayer/SoundEntryPlayer.h>
#include <noggit/ui/windows/EditorWindows/ZoneIntroMusicPickerWindow.h>
#include <noggit/ui/windows/EditorWindows/SoundEntryPickerWindow.h>
#include <noggit/ui/windows/EditorWindows/ZoneMusicPickerWindow.h>
#include <noggit/ui/tools/MapCreationWizard/Ui/MapCreationWizard.hpp>

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
#include <noggit/ui/tools/UiCommon/expanderwidget.h>


#include <iostream>
#include <sstream>
#include <string>
#include <bitset>

namespace Noggit
{
    namespace Ui
    {
        zone_id_browser::zone_id_browser(QWidget* parent)
            : QWidget(parent)
            , _area_tree(new QTreeWidget())
            , mapID(-1)
        {

            auto layout = new QFormLayout(this);

            _radius_spin = new QDoubleSpinBox(this);
            _radius_spin->setRange(0.0f, 1000.0f);
            _radius_spin->setDecimals(2);
            _radius_spin->setValue(_radius);

            layout->addRow("Radius:", _radius_spin);

            _radius_slider = new QSlider(Qt::Orientation::Horizontal, this);
            _radius_slider->setRange(0, 250);
            _radius_slider->setSliderPosition(_radius);

            QPushButton* edit_area_button = new QPushButton("Edit selected Area", this);
            edit_area_button->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::cog));
            QPushButton* add_zone_button = new QPushButton("Add a new Zone(parent Area)", this);
            add_zone_button->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
            QPushButton* add_subzone_button = new QPushButton("Add a new Subzone(selected as Parent)", this);
            add_subzone_button->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));


            _area_editor = new AreaEditor(this);

            layout->addRow(_radius_slider);
            layout->addRow(_area_tree);
            layout->addRow(edit_area_button);
            layout->addRow(add_zone_button);
            layout->addRow(add_subzone_button);

            setMinimumWidth(250);

            connect(_area_tree, &QTreeWidget::itemSelectionChanged
                , [this]
                {
                    auto const& selected_items = _area_tree->selectedItems();
                    if (selected_items.size())
                    {
                        emit selected(selected_items.back()->data(0, 1).toInt());
                    }
                }
            );

            connect(_area_tree, &QTreeWidget::itemDoubleClicked
                , [this]
                {
                    open_area_editor();
                });

            connect(_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged)
                , [&](double v)
                {
                    _radius = v;
                    QSignalBlocker const blocker(_radius_slider);
                    _radius_slider->setSliderPosition((int)std::round(v));
                }
            );

            connect(_radius_slider, &QSlider::valueChanged
                , [&](int v)
                {
                    _radius = v;
                    QSignalBlocker const blocker(_radius_spin);
                    _radius_spin->setValue(v);
                }
            );

            connect(edit_area_button, &QPushButton::clicked, [=]() {
                open_area_editor();
                });

            connect(add_zone_button, &QPushButton::clicked, [=]() {
                add_new_zone();
                });

            connect(add_subzone_button, &QPushButton::clicked, [=]() {
                add_new_subzone();
                });
        }

        void zone_id_browser::setMapID(int id)
        {
            mapID = id;

            for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
            {
                if (i->getInt(MapDB::MapID) == id)
                {
                    std::stringstream ss;
                    ss << id << "-" << i->getString(MapDB::InternalName);
                    _area_tree->setHeaderLabel(ss.str().c_str());
                }
            }

            buildAreaList();
        }

        void zone_id_browser::setZoneID(int id)
        {
            QSignalBlocker const block_area_tree(_area_tree);

            if (_items.find(id) != _items.end())
            {
                _area_tree->selectionModel()->clear();
                auto* item = _items.at(id);

                item->setSelected(true);

                while ((item = item->parent()))
                {
                    item->setExpanded(true);
                }
            }
        }

        void zone_id_browser::buildAreaList()
        {
            QSignalBlocker const block_area_tree(_area_tree);
            _area_tree->clear();
            _area_tree->setColumnCount(1);
            _items.clear();

            //  Read out Area List.
            for (DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i)
            {
                if (i->getInt(AreaDB::Continent) == mapID)
                {
                    add_area(i->getInt(AreaDB::AreaID));
                }
            }
        }

        void zone_id_browser::changeRadius(float change)
        {
            _radius_spin->setValue(_radius + change);
        }

        void zone_id_browser::setRadius(float radius)
        {
            _radius_spin->setValue(radius);
        }

        int zone_id_browser::GetSelectedAreaId()
        {
            auto const& selected_items = _area_tree->selectedItems();
            if (selected_items.size())
            {
                int selected_area_id = selected_items.back()->data(0, 1).toInt();
                return selected_area_id;
            }
            return 0;
        }

        QTreeWidgetItem* zone_id_browser::create_or_get_tree_widget_item(int area_id)
        {
            auto it = _items.find(area_id);

            if (it != _items.end())
            {
                return _items.at(area_id);
            }
            else
            {
                QTreeWidgetItem* item = new QTreeWidgetItem();

                std::stringstream ss;
                // ss << area_id << "-" << gAreaDB.getAreaName(area_id);
                std::string areaName = "";
                try
                {
                    AreaDB::Record rec = gAreaDB.getByID(area_id);
                    areaName = rec.getLocalizedString(AreaDB::Name);
                }
                catch (AreaDB::NotFound)
                {
                    areaName = "Unknown location";
                }
                ss << area_id << "-" << areaName;
                item->setData(0, 1, QVariant(area_id));
                item->setText(0, QString(ss.str().c_str()));
                _items.emplace(area_id, item);

                return item;
            }
        }

        QTreeWidgetItem* zone_id_browser::add_area(int area_id)
        {
            QTreeWidgetItem* item = create_or_get_tree_widget_item(area_id);

            std::uint32_t parent_area_id = gAreaDB.get_area_parent(area_id);

            if (parent_area_id && parent_area_id != area_id)
            {
                QTreeWidgetItem* parent_item = add_area(parent_area_id);
                parent_item->addChild(item);
            }
            else
            {
                _area_tree->addTopLevelItem(item);
            }

            return item;
        }

        void zone_id_browser::open_area_editor()
        {
            // opens and loads the area editor with the currently selected item in the tree.
            int selected_area_id = GetSelectedAreaId();
            if (selected_area_id != 0)
            {
                _area_editor->load_area(selected_area_id);
                _area_editor->show();
            }
            else
            {
                // No selected area popup
            }
        }

        void zone_id_browser::add_new_zone()
        {
            // create new zone area id 
            int new_id = gAreaDB.getEmptyRecordID();

            QDialog* zone_create_params = new QDialog(this);
            zone_create_params->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
            zone_create_params->setWindowTitle("New Zone settings");
            QVBoxLayout* zone_create_params_layout = new QVBoxLayout(zone_create_params);

            zone_create_params_layout->addWidget(new QLabel("AreaTable Id : ", zone_create_params));
            QSpinBox* params_id_spinbox = new QSpinBox(zone_create_params);
            params_id_spinbox->setRange(1, std::numeric_limits<int>::max());
            params_id_spinbox->setValue(new_id);
            zone_create_params_layout->addWidget(params_id_spinbox);

            zone_create_params_layout->addWidget(new QLabel("Zone Name : ", zone_create_params));
            QLineEdit* params_name_ledit = new QLineEdit(zone_create_params);
            params_name_ledit->setText("Unnamed Noggit Zone");
            zone_create_params_layout->addWidget(params_name_ledit);

            QPushButton* params_okay = new QPushButton("Create new Zone", zone_create_params);
            zone_create_params_layout->addWidget(params_okay);

            connect(params_okay, &QPushButton::clicked
                , [=]()
                {
                    // check if ID is valid
                    if (!gAreaDB.CheckIfIdExists(params_id_spinbox->value()))
                    {
                        zone_create_params->accept();
                    }
                    else
                    {
                        QMessageBox::warning
                        (nullptr
                            , "ID already in use"
                            , "Id is already used, use a different one"
                            , QMessageBox::Ok
                        );
                    }
                });

            // start popup and wait for ok button
            if (zone_create_params->exec() == QDialog::Accepted)
            {
                auto new_record = gAreaDB.addRecord(params_id_spinbox->value()); // could add a try catch but we already check if id is used before
                // init some defaults
                new_record.write(AreaDB::Continent, mapID);
                // get new areabit
                new_record.write(AreaDB::AreaBit, gAreaDB.get_new_areabit());
                unsigned int flags = 0;
                flags |= (1 << 6); // Allow Dueling
                new_record.write(AreaDB::Flags, flags);
                new_record.write(AreaDB::UnderwaterSoundProviderPreferences, 11); // underwater sound pref, usually set

                // locale stuff
                // new_record.writeString(AreaDB::Name, params_name_ledit->text().toStdString()); // only write default name for enUS and enGB ? maybe get the client's locale somehow
                int locale_id = Noggit::Application::NoggitApplication::instance()->clientData()->getLocaleId();
                new_record.writeLocalizedString(AreaDB::Name, params_name_ledit->text().toStdString(), locale_id);
                new_record.write(AreaDB::Name + 16, 16712190); // loc mask, only verified for enUS

                new_record.write(AreaDB::MinElevation, -500.0f);
                // save dbc instantly ?
                gAreaDB.save();
                // add to tree
                auto areawidgetitem = add_area(params_id_spinbox->value());
                // select the new item
                _area_tree->clearSelection();
                areawidgetitem->setSelected(true);
                open_area_editor();// open the editor
            }
        }

        void zone_id_browser::add_new_subzone()
        {
            // create new subzone area id 
            // set selected as parent
            int selected_areaid = GetSelectedAreaId();
            if (!selected_areaid) // no valid item selected
                return;
            // check if it's a valid parent : it shouldn't have a parent
            std::uint32_t selected_parent_area_id = gAreaDB.get_area_parent(selected_areaid); // the selected area's parentid
            if (selected_parent_area_id)
            {
                QMessageBox::information
                (nullptr
                    , "Wrong Parent type"
                    , "The parent must be a Zone, not a Subzone."
                    , QMessageBox::Ok
                );
                return;
            }

            int new_id = gAreaDB.getEmptyRecordID();

            QDialog* zone_create_params = new QDialog(this);
            zone_create_params->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
            zone_create_params->setWindowTitle("New Zone settings");
            QVBoxLayout* zone_create_params_layout = new QVBoxLayout(zone_create_params);

            zone_create_params_layout->addWidget(new QLabel("AreaTable Id : ", zone_create_params));
            QSpinBox* params_id_spinbox = new QSpinBox(zone_create_params);
            params_id_spinbox->setRange(1, std::numeric_limits<int>::max());
            params_id_spinbox->setValue(new_id);
            zone_create_params_layout->addWidget(params_id_spinbox);

            zone_create_params_layout->addWidget(new QLabel("Subzone Name : ", zone_create_params));
            QLineEdit* params_name_ledit = new QLineEdit(zone_create_params);
            params_name_ledit->setText("Unnamed Noggit Subzone");
            zone_create_params_layout->addWidget(params_name_ledit);

            QPushButton* params_okay = new QPushButton("Create new Subzone", zone_create_params);
            zone_create_params_layout->addWidget(params_okay);

            connect(params_okay, &QPushButton::clicked
                , [=]()
                {
                    // check if ID is valid
                    int id = params_id_spinbox->value();
                    bool exists = gAreaDB.CheckIfIdExists(params_id_spinbox->value());
                    if (exists)
                    {
                        QMessageBox::warning
                        (nullptr
                            , "ID already in use"
                            , "Id is already used, use a different one"
                            , QMessageBox::Ok
                        );
                    }
                    else
                    {
                        zone_create_params->accept();
                    }
                });

            // start popup and wait for ok button
            if (zone_create_params->exec() == QDialog::Accepted)
            {
                auto new_record = gAreaDB.addRecord(params_id_spinbox->value());
                // init some defaults
                new_record.write(AreaDB::Continent, mapID);

                new_record.write(AreaDB::Region, selected_areaid); // set selecetd area as parent.
                // get new areabit
                new_record.write(AreaDB::AreaBit, gAreaDB.get_new_areabit());
                unsigned int flags = 0;
                flags |= (1 << 6); // Allow Dueling
                flags |= (1 << 10); // force area on dynamic transport
                flags |= (1 << 14); // enable flight bounds
                flags |= (1 << 30); // subzone flag
                new_record.write(AreaDB::Flags, flags); // allow dueling + force area on dynamic transport + enable flight bounds+ subzone flags
                new_record.write(AreaDB::UnderwaterSoundProviderPreferences, 11); // underwater sound pref, usually set
                // lcoale stuff
                int locale_id = Noggit::Application::NoggitApplication::instance()->clientData()->getLocaleId();
                new_record.writeLocalizedString(AreaDB::Name, params_name_ledit->text().toStdString(), locale_id);
                new_record.write(AreaDB::Name + 16, 16712190); // loc mask, only verified for enUS

                new_record.write(AreaDB::MinElevation, -500.0f); // loc mask, only verified for enUS
                // save dbc instantly ?
                gAreaDB.save();
                // add to tree
                auto areawidgetitem = add_area(params_id_spinbox->value());
                // select the new item
                _area_tree->clearSelection();
                areawidgetitem->setSelected(true);
                open_area_editor();// open the editor
            }

        }

        AreaEditor::AreaEditor(QWidget* parent) 
        : QWidget(parent)
        {
            setWindowTitle("Area Editor");
            setWindowFlags(Qt::Dialog);
            // setWindowFlags(Qt::Widget);

            auto main_layout = new QHBoxLayout(this);

            QGroupBox* area_settings_group = new QGroupBox("Area Settings", this);
            main_layout->addWidget(area_settings_group);
            auto layout = new QFormLayout(area_settings_group);
            // main_layout->addLayout(layout);

            _area_id_label = new QLabel("0", this);
            _area_id_label->setEnabled(false);

            _parent_area_label = new QLabel("-NONE-", area_settings_group);
            _parent_area_label->setEnabled(false);

            // auto parent_set_layout = new QHBoxLayout(this);
            _set_parent_button = new QPushButton("Set Selected Zone as Parent", area_settings_group);
            // QPushButton* unset_parent_button = new QPushButton("Unset Parent", this);
            // parent_set_layout->addWidget(set_parent_button);
            // parent_set_layout->addWidget(unset_parent_button);

            _area_name = new Tools::MapCreationWizard::Ui::LocaleDBCEntry(area_settings_group);

            _exploration_level_spinbox = new QSpinBox(this);
            _exploration_level_spinbox->setRange(-1, 255);
            
            _ambiant_multiplier = new QSlider(Qt::Horizontal, this);
            _ambiant_multiplier->setRange(0, 100); // 0.0 - 1.0, decimal not supported so *100
            _ambiant_multiplier->setTickInterval(5);
            _ambiant_multiplier->setSingleStep(5);

            // faction group
            // read FactionGroup.dbc or just hardcode?
            _faction_group_combobox = new QComboBox(this);
            _faction_group_combobox->addItem("Contested"); // 0
            _faction_group_combobox->addItem("Alliance"); // 2
            _faction_group_combobox->addItem("Horde"); // 4
            _faction_group_combobox->addItem("Horde & Alliance"); // mask : 2 + 4

            // _liquid_type_water_combobox = new QComboBox(this);
            // _liquid_type_water_combobox->addItem("None");
            // _liquid_type_ocean_combobox = new QComboBox(this);
            // _liquid_type_ocean_combobox->addItem("None");
            // _liquid_type_magma_combobox = new QComboBox(this);
            // _liquid_type_magma_combobox->addItem("None");
            // _liquid_type_slime_combobox = new QComboBox(this);
            // _liquid_type_slime_combobox->addItem("None");
            // for (DBCFile::Iterator i = gLiquidTypeDB.begin(); i != gLiquidTypeDB.end(); ++i)
            // {
            //     std::stringstream ss;
            //     int liquid_type = i->getInt(LiquidTypeDB::Type);
            //     ss << i->getInt(LiquidTypeDB::ID) << "-" << i->getString(LiquidTypeDB::Name);
            //     switch (liquid_type)
            //     {
            //     case 0:
            //         _liquid_type_water_combobox->addItem(i->getString(LiquidTypeDB::Name));
            //         break;
            //     case 1:
            //         _liquid_type_ocean_combobox->addItem(i->getString(LiquidTypeDB::Name));
            //         break;
            //     case 2:
            //         _liquid_type_magma_combobox->addItem(i->getString(LiquidTypeDB::Name));
            //         break;
            //     case 3:
            //         _liquid_type_slime_combobox->addItem(i->getString(LiquidTypeDB::Name));
            //     }
            // }

            _sound_provider_preferences_cbbox = new QComboBox(this);
            _sound_provider_preferences_cbbox->addItem("None");

            _underwater_sound_provider_preferences_cbbox = new QComboBox(this);
            _underwater_sound_provider_preferences_cbbox->addItem("None");
            for (DBCFile::Iterator i = gSoundProviderPreferencesDB.begin(); i != gSoundProviderPreferencesDB.end(); ++i)
            {
                std::stringstream ss;
                ss << i->getInt(SoundProviderPreferencesDB::ID) << "-" << i->getString(SoundProviderPreferencesDB::Description);

                _sound_provider_preferences_cbbox->addItem(ss.str().c_str());
                _underwater_sound_provider_preferences_cbbox->addItem(ss.str().c_str());
            }

            _sound_ambiance_day_button = new QPushButton("-None-", this);
            _sound_ambiance_day_button->setProperty("id", 0);
            connect(_sound_ambiance_day_button, &QPushButton::clicked, [=]() {
                auto window = new SoundEntryPickerWindow(_sound_ambiance_day_button, SoundEntryTypes::ZONE_AMBIENCE, false, this);
                window->show();
                });

            _sound_ambiance_night_button = new QPushButton("-None-", this);
            _sound_ambiance_night_button->setProperty("id", 0);
            connect(_sound_ambiance_night_button, &QPushButton::clicked, [=]() {
                auto window = new SoundEntryPickerWindow(_sound_ambiance_night_button, SoundEntryTypes::ZONE_AMBIENCE, false, this);
                window->show();
                });

            _zone_music_button = new QPushButton("-None-", this);
            _zone_music_button->setProperty("id", 0);
            connect(_zone_music_button, &QPushButton::clicked, [=]() {
                auto window = new ZoneMusicPickerWindow(_zone_music_button, this);
                window->show();
                });

            _zone_intro_music_button = new QPushButton("-None-", this);
            _zone_intro_music_button->setProperty("id", 0);
            connect(_zone_intro_music_button, &QPushButton::clicked, [=]() {
                auto window = new ZoneIntroMusicPickerWindow(_zone_intro_music_button, this);
                window->show();
                });

            // advanced settings group
            auto* AdvancedOptionsBox = new ExpanderWidget(this);
            AdvancedOptionsBox->setExpanderTitle("Advanced Options");
            // selectionOptionsBox->setExpanded(_settings->value("object_editor/movement_options", false).toBool());

            auto advancedOptionsBox_content = new QWidget(this);
            auto advancedOptions_layout = new QFormLayout(advancedOptionsBox_content);
            // advancedOptions_layout->setAlignment(Qt::AlignTop);
            AdvancedOptionsBox->addPage(advancedOptionsBox_content);
            AdvancedOptionsBox->setExpanded(false);

            _min_elevation_spinbox = new QDoubleSpinBox(this);
            _min_elevation_spinbox->setRange(-5000, 5000); // only 3 known values : -5000, -500, 1000
            _min_elevation_spinbox->setSingleStep(100);

            advancedOptions_layout->addRow("Min Elevation:", _min_elevation_spinbox);
            advancedOptions_layout->addRow("Ambiant Multiplier:", _ambiant_multiplier);
            // advancedOptions_layout->addRow("Water Liquid Type override:", _liquid_type_water_combobox);
            // advancedOptions_layout->addRow("Ocean Liquid Type override:", _liquid_type_ocean_combobox);
            // advancedOptions_layout->addRow("Magma Liquid Type override:", _liquid_type_magma_combobox);
            // advancedOptions_layout->addRow("Slime Liquid Type override:", _liquid_type_slime_combobox);
            advancedOptions_layout->addRow("Sound Provider Preference:", _sound_provider_preferences_cbbox);
            advancedOptions_layout->addRow("Underwater Sound Provider Preference:", _underwater_sound_provider_preferences_cbbox);


            QPushButton* save_area_button = new QPushButton("Save changes (write DBC)", this);
            // TODO : unset parent button?

            // flags tab **************************//

            QGroupBox* area_flags_group = new QGroupBox("Flags", this);
            main_layout->addWidget(area_flags_group);
            auto flags_layout = new QVBoxLayout(area_flags_group);

            _flags_value_spinbox = new QSpinBox(this);
            _flags_value_spinbox->setRange(0, INT_MAX); // uint not necessary for wrath because we only have 31 flags. 
                                                            // for cata+ we'll need all 32 bits instead, might need a line edit
            flags_layout->addWidget(_flags_value_spinbox);

            // TODO : update checkboxes when value is changed, temporarly disable it from edit
            _flags_value_spinbox->setEnabled(false);

            for (int i = 0; i < 31; ++i)
            {
                QCheckBox* flag_checkbox = new QCheckBox(QString::fromStdString(area_flags_names.at(i)), this);
                flags_checkboxes[i] = flag_checkbox;

                flags_layout->addWidget(flag_checkbox);

                connect(flag_checkbox, &QCheckBox::stateChanged, [&, i](bool state) {
                // connect(flag_checkbox, &QCheckBox::clicked, [=]() {
                    // int old_value = _flags_value_spinbox->value();
                    int new_value = _flags_value_spinbox->value();
                    if (state) // set bit
                        new_value |= (1U << (i));
                    else // remove bit
                        new_value &= ~(1U << (i));
                        
                    _flags_value_spinbox->setValue(new_value);
                    });
            }
            // hide some useless flags to gain space
            flags_checkboxes[30]->setEnabled(false); // user can't set subzone flag.
            flags_checkboxes[20]->setHidden(true); // tournement realm thingy, useless for pservers
            flags_checkboxes[17]->setHidden(true); // "Area not in use", prob some blizz dev stuff
            //************************************//
            layout->addRow("Area ID:", _area_id_label);
            layout->addRow("Area Name:", _area_name);
            layout->addWidget(_set_parent_button);
            layout->addRow("Parent Area ID:", _parent_area_label);
            layout->addRow("Faction Group:", _faction_group_combobox);
            layout->addRow("Exploration Level:", _exploration_level_spinbox);
            layout->addRow("Zone Music:", _zone_music_button);
            layout->addRow("Zone Intro Music:", _zone_intro_music_button);
            layout->addRow("Sound Ambience Day:", _sound_ambiance_day_button);
            layout->addRow("Sound Ambience Night:", _sound_ambiance_night_button);
            layout->addRow(new QLabel("If only day or night is set but not the other, the other will be saved as the same sound."));

            layout->addRow(AdvancedOptionsBox);
            layout->addRow(save_area_button);

            layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

            // main_layout->addStretch();

            connect(_set_parent_button, &QPushButton::clicked, [=]() {

                // Current design choice : Cannot change a zone to be a subzone or the opposite, only current subzones can have a parent set.
                auto parent = static_cast<zone_id_browser*>(this->parentWidget());
                auto tree_selected_id = parent->GetSelectedAreaId();
                if (!tree_selected_id)
                    return;
                // can only set a zone as parent, not a subzone.
                // a zone must be a top level item, and not have a parent or the subzone flag.

                // checks :
                // 1 : current area must be a subzone
                if (!_parent_area_id)
                    return;

                std::uint32_t selected_parent_area_id = gAreaDB.get_area_parent(tree_selected_id); // the selected area's parentid
                if (selected_parent_area_id)
                {
                    // error, parent must not have a parent
                    QMessageBox messagebox;
                    messagebox.setIcon(QMessageBox::Information);
                    messagebox.setWindowIcon(QIcon(":/icon"));
                    messagebox.setWindowTitle("Wrong Parent type");
                    messagebox.setText("The parent must be a Zone, not a Subzone.");
                    messagebox.exec();
                    return;
                }
                // can't be self
                if (tree_selected_id == _parent_area_id)
                    return;// same parent, ignore
                if (tree_selected_id == _area_id_label->text().toInt())
                    return; // error, can't select self
                
                // save the change for the session (don't write dbc yet)
                _parent_area_id = tree_selected_id;
                std::stringstream ss;
                ss << _parent_area_id << "-" << gAreaDB.getAreaFullName(_parent_area_id);
                _parent_area_label->setText(ss.str().c_str());

                // _parent_area_label->setText(std::to_string( tree_selected_id).c_str());
                try
                {
                    auto curr_record = gAreaDB.getByID(_area_id_label->text().toInt());
                    curr_record.write(AreaDB::Region, tree_selected_id);
                    // update the tree
                    parent->buildAreaList();
                    // select the item ?
                    auto item = parent->create_or_get_tree_widget_item(_area_id_label->text().toInt());
                    // parent->selected(_area_id_label->text().toInt());
                }
                catch (AreaDB::NotFound)
                {

                }

                });

            connect(save_area_button, &QPushButton::clicked, [=]() {
                save_area();// save and write DBC
                });

            connect(_flags_value_spinbox, qOverload<int>(&QSpinBox::valueChanged), [&](int v) {

                // std::bitset<32> IntBits(_flags_value_spinbox->value());
                // 
                // for (int i = 0; i < 31; i++)
                //     flags_checkboxes[i]->setChecked(IntBits[i]);
                });
        }

        void AreaEditor::load_area(int area_id)
        {
            try
            {
                DBCFile::Record record = gAreaDB.getByID(area_id);

                _area_id_label->setText(QString(std::to_string(area_id).c_str()));

                _areabit = record.getInt(AreaDB::AreaBit);

                _parent_area_id = record.getInt(AreaDB::Region);

                if (_parent_area_id)
                {
                    std::stringstream ss;
                    ss << _parent_area_id << "-" << gAreaDB.getAreaFullName(_parent_area_id);
                    _parent_area_label->setText(ss.str().c_str());
                }
                else
                    _parent_area_label->setText("-NONE-");

                // hide some UI if not subzone
                _set_parent_button->setHidden(_parent_area_id ? false : true);

                // _area_name_ledit->setText(record.getString(AreaDB::Name));
                _area_name->fill(record, AreaDB::Name);

                _flags_value_spinbox->setValue(record.getInt(AreaDB::Flags));

                std::bitset<32> IntBits(_flags_value_spinbox->value());
                for (int i = 0; i < 31; i++)
                    flags_checkboxes[i]->setChecked(IntBits[i]);

                _exploration_level_spinbox->setValue(record.getInt(AreaDB::ExplorationLevel));

                int faction_group_mask = record.getInt(AreaDB::FactionGroup);
                switch (faction_group_mask) // hardcoded but can be read from factiongroup.dbc
                {
                    case 2:
                        _faction_group_combobox->setCurrentIndex(1);
                        break;
                    case 4:
                            _faction_group_combobox->setCurrentIndex(2);
                            break;
                    case 6:
                        _faction_group_combobox->setCurrentIndex(3);
                        break;
                    default:
                        _faction_group_combobox->setCurrentIndex(0);
                }

                _min_elevation_spinbox->setValue(record.getFloat(AreaDB::MinElevation)); // only 3 known values : -5000, -500, 1000

                _ambiant_multiplier->setValue(record.getFloat(AreaDB::AmbientMultiplier) * 100);

                int sound_provider_id = record.getInt(AreaDB::SoundProviderPreferences);
                if (sound_provider_id != 0 && gSoundProviderPreferencesDB.CheckIfIdExists(sound_provider_id))
                {
                    int row_id = gSoundProviderPreferencesDB.getRecordRowId(sound_provider_id);
                    _sound_provider_preferences_cbbox->setCurrentIndex(row_id + 1); // index 0 = "None"
                }
                else
                    _sound_provider_preferences_cbbox->setCurrentIndex(0);

                int underwater_sound_provider_id = record.getInt(AreaDB::UnderwaterSoundProviderPreferences);
                if (underwater_sound_provider_id != 0 && gSoundProviderPreferencesDB.CheckIfIdExists(underwater_sound_provider_id))
                {
                    int row_id = gSoundProviderPreferencesDB.getRecordRowId(underwater_sound_provider_id);
                    _underwater_sound_provider_preferences_cbbox->setCurrentIndex(row_id + 1); // index 0 = "None"
                }
                else
                    _underwater_sound_provider_preferences_cbbox->setCurrentIndex(0);

                int zone_music_id = record.getInt(AreaDB::ZoneMusic);
                if (zone_music_id != 0 && gZoneMusicDB.CheckIfIdExists(zone_music_id))
                {
                    _zone_music_button->setProperty("id", zone_music_id);
                    auto zone_music_record = gZoneMusicDB.getByID(zone_music_id);
                    std::stringstream ss;
                    ss << zone_music_id << "-" << zone_music_record.getString(ZoneMusicDB::Name);
                    _zone_music_button->setText(ss.str().c_str());
                }
                else
                {
                    _zone_music_button->setText("-NONE-");
                    _zone_music_button->setProperty("id", 0);
                }

                int zone_intro_music_id = record.getInt(AreaDB::ZoneIntroMusicTable);
                if (zone_intro_music_id != 0 && gZoneIntroMusicTableDB.CheckIfIdExists(zone_intro_music_id))
                {
                    _zone_intro_music_button->setProperty("id", zone_intro_music_id);
                    auto zone_intro_music_record = gZoneIntroMusicTableDB.getByID(zone_intro_music_id);
                    std::stringstream ss;
                    ss << zone_intro_music_id << "-" << zone_intro_music_record.getString(ZoneIntroMusicTableDB::Name);
                    _zone_intro_music_button->setText(ss.str().c_str());
                }
                else
                {
                    _zone_intro_music_button->setProperty("id", 0);
                    _zone_intro_music_button->setText("-NONE-");
                }

                int sound_ambiance_id = record.getInt(AreaDB::SoundAmbience);
                if (sound_ambiance_id != 0 && gSoundAmbienceDB.CheckIfIdExists(sound_ambiance_id))
                {
                    auto sound_ambiance_record = gSoundAmbienceDB.getByID(sound_ambiance_id);

                    int day_sound_id = sound_ambiance_record.getInt(SoundAmbienceDB::SoundEntry_day);
                    if (day_sound_id != 0 && gSoundEntriesDB.CheckIfIdExists(day_sound_id))
                    {
                        auto sound_entry_day_record = gSoundEntriesDB.getByID(day_sound_id);
                        std::stringstream ss_day;
                        ss_day << day_sound_id << "-" << sound_entry_day_record.getString(SoundEntriesDB::Name);
                        _sound_ambiance_day_button->setText(ss_day.str().c_str());
                        _sound_ambiance_day_button->setProperty("id", day_sound_id);
                    }
                    else
                    {
                        _sound_ambiance_day_button->setText("-NONE-");
                        _sound_ambiance_day_button->setProperty("id", 0);
                    }

                    int night_sound_id = sound_ambiance_record.getInt(SoundAmbienceDB::SoundEntry_night);
                    if (night_sound_id != 0 && gSoundEntriesDB.CheckIfIdExists(night_sound_id))
                    {
                        auto sound_entry_night_record = gSoundEntriesDB.getByID(night_sound_id);
                        std::stringstream ss_night;
                        ss_night << night_sound_id << "-" << sound_entry_night_record.getString(SoundEntriesDB::Name);
                        _sound_ambiance_night_button->setText(ss_night.str().c_str());
                        _sound_ambiance_night_button->setProperty("id", night_sound_id);
                    }
                    else
                    {
                        _sound_ambiance_night_button->setText("-NONE-");
                        _sound_ambiance_night_button->setProperty("id", 0);
                    }
                }
                else
                {
                    _sound_ambiance_day_button->setProperty("id", 0);
                    _sound_ambiance_day_button->setText("-NONE-");
                    _sound_ambiance_night_button->setProperty("id", 0);
                    _sound_ambiance_night_button->setText("-NONE-");
                }
            }
            catch (AreaDB::NotFound)
            {

            }
        }

        void AreaEditor::save_area()
        {
            try
            {
                DBCFile::Record record = gAreaDB.getByID(_area_id_label->text().toInt()); // is_new_record ? gLightDB.addRecord(Id) : gLightDB.getByID(Id);
                // record.write(AreaDB::ID, entry_id);
                // rewrite mapid ?
                record.write(AreaDB::Region, _parent_area_id);
                record.write(AreaDB::AreaBit, _areabit);
                record.write(AreaDB::Flags, _flags_value_spinbox->value());

                int SoundProviderPreferences_id = 0;
                if (_sound_provider_preferences_cbbox->currentIndex() != 0)
                {
                    auto rec = gSoundProviderPreferencesDB.getRecord(_sound_provider_preferences_cbbox->currentIndex() - 1);
                    SoundProviderPreferences_id = rec.getInt(SoundProviderPreferencesDB::ID);
                }
                record.write(AreaDB::SoundProviderPreferences, SoundProviderPreferences_id);
                
                int underwaterSoundProviderPreferences_id = 0;
                if (_underwater_sound_provider_preferences_cbbox->currentIndex() != 0)
                {
                    auto rec = gSoundProviderPreferencesDB.getRecord(_underwater_sound_provider_preferences_cbbox->currentIndex() - 1);
                    underwaterSoundProviderPreferences_id = rec.getInt(SoundProviderPreferencesDB::ID);
                }
                record.write(AreaDB::UnderwaterSoundProviderPreferences, underwaterSoundProviderPreferences_id);

                // Ambiance ID. Blizzard stores those as unamed pair
                // Just iterate the DBC to see if an entry with our settings already exists, if not create it.
                // The reasoning for not having a selector/picker with the existing pairs is that it has less freedom, is harder to use and it's ugly if they don't have a name.
                // This doesn't have the option to edit that entry for all its users though.

                int soundambience_day = _sound_ambiance_day_button->property("id").toInt();
                int soundambience_night = _sound_ambiance_night_button->property("id").toInt();

                if (soundambience_day && !soundambience_night) // if day is set but not night, set night to day
                    soundambience_night = soundambience_day;
                else if (!soundambience_day && soundambience_night) // night to day
                    soundambience_night = soundambience_day;

                if (soundambience_day && soundambience_night) // check if both day and night are set
                {
                    bool sound_ambiance_exists {false};
                    for (DBCFile::Iterator i = gSoundAmbienceDB.begin(); i != gSoundAmbienceDB.end(); ++i)
                    {
                        int day_id = i->getInt(SoundAmbienceDB::SoundEntry_day);
                        int night_id = i->getInt(SoundAmbienceDB::SoundEntry_night);
                        if (day_id == soundambience_day && night_id == soundambience_night)
                        {
                            record.write(AreaDB::SoundAmbience, i->getInt(SoundAmbienceDB::ID));
                            sound_ambiance_exists = true;
                            break;
                        }
                    }
                    if (!sound_ambiance_exists)
                    {
                        // create new sond entry record with the two ids
                        auto new_id = gSoundAmbienceDB.getEmptyRecordID();
                        auto new_record = gSoundAmbienceDB.addRecord(new_id);

                        new_record.write(SoundAmbienceDB::SoundEntry_day, soundambience_day);
                        new_record.write(SoundAmbienceDB::SoundEntry_night, soundambience_night);
                        gSoundAmbienceDB.save();
                        record.write(AreaDB::SoundAmbience, new_id);
                    }
                }
                else
                    record.write(AreaDB::SoundAmbience, 0); // if night or day isn't set, set to 0


                record.write(AreaDB::ZoneMusic, _zone_music_button->property("id").toInt());
                record.write(AreaDB::ZoneIntroMusicTable, _zone_intro_music_button->property("id").toInt());

                record.write(AreaDB::ExplorationLevel, _exploration_level_spinbox->value());

                _area_name->toRecord(record, AreaDB::Name);
                // update name in the tree
                auto parent = static_cast<zone_id_browser*>(this->parentWidget());
                auto item = parent->create_or_get_tree_widget_item(_area_id_label->text().toInt());
                std::stringstream ss;
                std::string areaName = record.getLocalizedString(AreaDB::Name);
                ss << _area_id_label->text().toInt() << "-" << areaName;
                item->setText(0, QString(ss.str().c_str()));

                record.write(AreaDB::FactionGroup, _faction_group_combobox->currentIndex() * 2);
                record.write(AreaDB::MinElevation, static_cast<float>(_min_elevation_spinbox->value()));
                record.write(AreaDB::AmbientMultiplier, _ambiant_multiplier->value() / 100.0f);
                record.write(AreaDB::LightId, 0); // never used

                gAreaDB.save();

                load_area(_area_id_label->text().toInt());// reload ui, especially for night/day ambience
            }
            catch (AreaDB::NotFound)
            {

            } 
        }

    }

}
