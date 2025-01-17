// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;

namespace Noggit
{
    namespace Ui
    {
        class ZoneMusicPickerWindow : public QWidget
        {
            Q_OBJECT
        public:
            ZoneMusicPickerWindow(QPushButton* button, QWidget* parent = nullptr);

        private:
            QLineEdit* _tree_searchbar;
            QListWidget* _picker_listview;

            int _entry_id = 0;
            QLabel* _entry_id_lbl;
            QLineEdit* _name_ledit;
            QSpinBox* _day_min_interval_spinbox;
            QSpinBox* _day_max_interval_spinbox;
            QSpinBox* _night_min_interval_spinbox;
            QSpinBox* _night_max_interval_spinbox;

            QPushButton* _day_music_button;
            QPushButton* _night_music_button;

            void select_entry(int id);
            void save_entry(int entry_id);
            // void duplicate_entry();
        };
    }
}

