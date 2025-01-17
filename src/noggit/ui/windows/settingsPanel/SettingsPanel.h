// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QMainWindow>

namespace Ui
{
  class SettingsPanel;
}

class QSettings;

namespace Noggit
{
  namespace Ui
  {
    class settings : public QMainWindow
    {
      Q_OBJECT
      QSettings* _settings;
      ::Ui::SettingsPanel* ui;
    public:
      settings(QWidget* parent = nullptr);
      void discard_changes();
      void save_changes();

    signals:
      void saved();
    };
  }
}
