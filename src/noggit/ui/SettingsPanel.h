// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <qt-color-widgets/color_selector.hpp>

#include <QtCore/QSettings>
#include <QDialog>
#include <ui_SettingsPanel.h>


namespace noggit
{
  namespace ui
  {
    class settings : public QDialog
    {
      QSettings* _settings;
      Ui::SettingsPanel* SettingsPanelUi;
    public:
      settings(QWidget* parent = nullptr);
      void discard_changes();
      void save_changes();
    };
  }
}
