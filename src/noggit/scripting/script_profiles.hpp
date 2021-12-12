// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

namespace Noggit {
  namespace Scripting {
    class scripting_tool;
    class script_settings;

    class script_profiles : public QGroupBox {
    public:
      script_profiles(Noggit::Scripting::scripting_tool * tool);
      void select_profile(int profile);
      std::string get_cur_profile();
      void clear();
      void add_profile(std::string const& profile);
      int profile_count();
      std::string get_profile(int index);
    private:
      Noggit::Scripting::scripting_tool* _tool;
      QGridLayout* _select_column;
      QComboBox* _selection;
      QLineEdit* _name_entry;
      QPushButton* _remove_button;
      QPushButton* _create_button;
      std::string _cur_profile;
      void on_change_script(int selection);
      void select_profile_gui(int profile);
    };
  }
}