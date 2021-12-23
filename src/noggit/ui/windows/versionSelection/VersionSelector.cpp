// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/windows/versionSelection/VersionSelector.h>
#include <noggit/ui/windows/mainWindow/main_window.hpp>
namespace Noggit
{
  namespace Ui
  {
    versionSelector::versionSelector(main_window* parent) : QDialog(parent)
    {
      setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
      ::Ui::VersionSelector versionSelector;
     
      versionSelector.setupUi(this);

      activateWindow();

      connect(versionSelector.select_shadowlands_version, &QPushButton::clicked
          , [=]
          {
              parent->setEnabled(true);
              parent->build_menu(true);
              hide();
          }
      );

      connect(versionSelector.select_wrath_version, &QPushButton::clicked
          , [=]
          {
              parent->setEnabled(true);
              parent->build_menu(false);
              hide();
          }
      );
    }
  }
}
