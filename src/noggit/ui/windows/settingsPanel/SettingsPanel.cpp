// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/windows/settingsPanel/SettingsPanel.h>
#include <noggit/Log.h>

#include <noggit/TextureManager.h>
#include <util/qt/overload.hpp>
#include <noggit/ui/FramelessWindow.hpp>
#include <sstream>

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QComboBox>
#include <QDir>
#include <QApplication>

#ifdef USE_MYSQL_UID_STORAGE
#include <mysql/mysql.h>
#endif

#include <ui_SettingsPanel.h>
#include <ui_TitleBar.h>


#include <algorithm>
#include <qmessagebox.h>


namespace Noggit
{
  namespace Ui
  {
    settings::settings(QWidget *parent) : QMainWindow(parent, Qt::Window), _settings(new QSettings(this))
    {
      auto body = new QWidget(this);
      ui = new ::Ui::SettingsPanel;
      ui->setupUi(body);
      setCentralWidget(body);
      setWindowTitle("Settings");

      auto titlebar = new QWidget(this);
      setupFramelessWindow(titlebar, this, minimumSize(), maximumSize(), false);
      setMenuWidget(titlebar);

      setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowStaysOnTopHint);

      connect(ui->importPathField, &QLineEdit::textChanged, [&](QString value)
              {
                _settings->setValue("project/import_file", value);
              }
      );


      connect(ui->importPathField_browse, &QPushButton::clicked, [=]
              {
                auto result(QFileDialog::getOpenFileName(
                    nullptr, "Import File Path", ui->importPathField->text()));

                if (!result.isNull())
                {
                  ui->importPathField->setText(result);
                }
              }
      );

      connect(ui->wmvLogPathField, &QLineEdit::textChanged, [&](QString value)
              {
                _settings->setValue("project/import_file", value);
              }
      );


      connect(ui->wmvLogPathField_browse, &QPushButton::clicked, [=]
              {
                auto result(QFileDialog::getOpenFileName(
                    nullptr, "WMV Log Path", ui->wmvLogPathField->text()));

                if (!result.isNull())
                {
                  ui->wmvLogPathField->setText(result);
                }
              }
      );


#ifdef USE_MYSQL_UID_STORAGE
      ui->MySQL_box->setEnabled(true);
      ui->MySQL_box->setCheckable(true);
      ui->mysql_warning->setVisible(false);
#endif

      ui->_theme->addItem("System");

      QDir theme_dir = QDir("./themes/");
      if (theme_dir.exists())
      {
        for (auto dir : theme_dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        {
          if (QDir(theme_dir.path() + "/" + dir).exists("theme.qss"))
          {
            ui->_theme->addItem(dir);
          }
        }
      }
      else
      {
        LogError
            << "Failed to load themes. The \"themes/\" folder does not exist in Noggit directory. Using system theme."
            << std::endl;
      }

      connect(ui->_theme, &QComboBox::currentTextChanged, [&](QString s)
              {
                if (s == "System")
                {
                  qApp->setStyleSheet("");
                  return;
                }

                auto sstream = std::stringstream();
                sstream << "./themes/" << s.toStdString() << "/theme.qss";

                QFile file(sstream.str().c_str());
                if (file.open(QFile::ReadOnly))
                {
                  QString style_sheet = QLatin1String(file.readAll());
                  QString style_sheet_fixed = style_sheet.replace("@rpath", QCoreApplication::applicationDirPath());

                  if (style_sheet_fixed.endsWith("/"))
                    style_sheet_fixed.chop(1);
                  else if (style_sheet_fixed.endsWith("\\"))
                    style_sheet_fixed.chop(2);

                  qApp->setStyleSheet(style_sheet_fixed);
                }
              }
      );

      connect(ui->_fps_limit_slider, &QSlider::valueChanged, [&](int value)
              {
                  ui->_fps_limit_current_label->setText(
                      QString(tr("FPS limitation, current : %1")).arg(value));
              });

      connect(ui->_adt_unload_dist, qOverload<int>(&QSpinBox::valueChanged)
          , [&](int v)
          {
              QSignalBlocker const blocker(ui->_adt_loading_radius);
              ui->_adt_loading_radius->setMaximum(v - 1);
          });

      connect(ui->_adt_loading_radius, qOverload<int>(&QSpinBox::valueChanged)
          , [&](int v)
          {
              QSignalBlocker const blocker(ui->_adt_unload_dist);
              ui->_adt_unload_dist->setMinimum(v + 1);
          });

      ui->_wireframe_color->setColor(Qt::white);

      connect(ui->saveButton, &QPushButton::clicked, [this]
              {
                hide();
                save_changes();
              }
      );

      connect(ui->discardButton, &QPushButton::clicked, [this]
              {
                hide();
                discard_changes();
              }
      );

      connect(ui->mysql_connect_test, &QPushButton::clicked, [this]
          {
              save_changes();
              #ifdef USE_MYSQL_UID_STORAGE
              mysql::testConnection();
              #endif
          }
      );

      // load the values in the fields
      discard_changes();

      // ui->_adt_loading_radius->setMaximum(ui->_adt_unload_dist->value() - 1);
      // ui->_adt_unload_dist->setMinimum(ui->_adt_loading_radius->value() + 1);
    }

    void settings::discard_changes()
    {
      ui->importPathField->setText(_settings->value("project/import_file", "import.txt").toString());
      ui->wmvLogPathField->setText(_settings->value("project/wmv_log_file").toString());
      ui->_view_distance->setValue(_settings->value("view_distance", 2000.f).toFloat());
      ui->_fov->setValue(_settings->value("fov", 54.f).toFloat());
      ui->_undock_tool_properties->setChecked(
          _settings->value("undock_tool_properties/enabled", true).toBool());
      ui->_undock_small_texture_palette->setChecked(
          _settings->value("undock_small_texture_palette/enabled", true).toBool());
      ui->_vsync_cb->setChecked(_settings->value("vsync", false).toBool());
      ui->_anti_aliasing_cb->setChecked(_settings->value("anti_aliasing", false).toBool());
      ui->_fullscreen_cb->setChecked(_settings->value("fullscreen", false).toBool());
      ui->_background_fps_limit_cb->setChecked(_settings->value("background_fps_limit", true).toBool());
      ui->_directional_light_cb->setChecked(_settings->value("directional_lightning", true).toBool());
      ui->_adt_unload_dist->setValue(_settings->value("unload_dist", 5).toInt());
      ui->_adt_unload_check_interval->setValue(_settings->value("unload_interval", 30).toInt());
      ui->_adt_loading_radius->setValue(_settings->value("loading_radius", 2).toInt());
      ui->_uid_cb->setChecked(_settings->value("uid_startup_check", true).toBool());
      ui->_load_fav_cb->setChecked(_settings->value("auto_load_fav_project", true).toBool());
      ui->_systemWindowFrame->setChecked(_settings->value("systemWindowFrame", true).toBool());
      ui->_nativeMenubar->setChecked(_settings->value("nativeMenubar", true).toBool());
      ui->_classic_ui->setChecked(_settings->value("classicUI", false).toBool());
      ui->_additional_file_loading_log->setChecked(
          _settings->value("additional_file_loading_log", false).toBool());
      ui->_keyboard_locale->setCurrentText(_settings->value("keyboard_locale", "QWERTY").toString());
      ui->_use_mclq_liquids_export->setChecked(_settings->value("use_mclq_liquids_export", false).toBool());
      ui->_theme->setCurrentText(_settings->value("theme", "Dark").toString());

      ui->assetBrowserBgCol->setColor(_settings->value("assetBrowser/background_color",
        QVariant::fromValue(QColor(127, 127, 127))).value<QColor>());
      ui->assetBrowserDiffuseLight->setColor(_settings->value("assetBrowser/diffuse_light",
        QVariant::fromValue(QColor::fromRgbF(1.0f, 0.532352924f, 0.0f))).value<QColor>());

      ui->assetBrowserAmbientLight->setColor(_settings->value("assetBrowser/ambient_light",
        QVariant::fromValue(QColor::fromRgbF(0.407770514f, 0.508424163f, 0.602650642f))).value<QColor>());

      ui->assetBrowserCopyToClipboard->setChecked(_settings->value("assetBrowser/copy_to_clipboard", true).toBool());
      ui->assetBrowserDefaultModel->setText(_settings->value("assetBrowser/default_model",
                                     "world/wmo/azeroth/human/buildings/human_farm/farm.wmo").toString());
      ui->assetBrowserMoveSensitivity->setValue(_settings->value("assetBrowser/move_sensitivity", 15.0f).toFloat());
      ui->assetBrowserRenderAssetPreview->setChecked(_settings->value("assetBrowser/render_asset_preview", false).toBool());


#ifdef USE_MYSQL_UID_STORAGE
      ui->MySQL_box->setChecked (_settings->value ("project/mysql/enabled").toBool());

      auto server_str = _settings->value("project/mysql/server", "127.0.0.1").toString();
      auto user_str = _settings->value("project/mysql/user", "127.0.0.1").toString();
      auto pwd_str = _settings->value("project/mysql/pwd", "127.0.0.1").toString();
      auto db_str = _settings->value("project/mysql/db", "127.0.0.1").toString();
      auto port_int = _settings->value("project/mysql/port", "127.0.0.1").toInt();

      // set some default
      if (server_str.isEmpty())
          server_str = "127.0.0.1";
      if (user_str.isEmpty())
          user_str = "root";
      if (pwd_str.isEmpty())
          pwd_str = "root";
      if (db_str.isEmpty())
          db_str = "noggit";
      if (!port_int)
          port_int = 3306;

      ui->_mysql_server_field->setText (server_str);
      ui->_mysql_user_field->setText(user_str);
      ui->_mysql_pwd_field->setText (pwd_str);
      ui->_mysql_db_field->setText (db_str);
      ui->_mysql_port_field->setValue (port_int);
#endif

      bool wireframe_type = _settings->value("wireframe/type", false).toBool();

      if (wireframe_type)
      {
        ui->radio_wire_cursor->setChecked(true);
      }
      else
      {
        ui->radio_wire_full->setChecked(true);
      }

      ui->_wireframe_radius->setValue(_settings->value("wireframe/radius", 1.5f).toFloat());
      ui->_wireframe_width->setValue(_settings->value("wireframe/width", 1.f).toFloat());
      ui->_wireframe_color->setColor(_settings->value("wireframe/color").value<QColor>());
      ui->_fps_limit_slider->setValue(_settings->value("fps_limit", 60).toInt());
    }

    void settings::save_changes()
    {
      _settings->setValue("project/import_file", ui->importPathField->text());
      _settings->setValue("project/wmv_log_file", ui->wmvLogPathField->text());
      _settings->setValue("view_distance", ui->_view_distance->value());
      _settings->setValue("fov", ui->_fov->value());
      _settings->setValue("undock_tool_properties/enabled", ui->_undock_tool_properties->isChecked());
      _settings->setValue("undock_small_texture_palette/enabled",
                          ui->_undock_small_texture_palette->isChecked());
      _settings->setValue("vsync", ui->_vsync_cb->isChecked());
      _settings->setValue("anti_aliasing", ui->_anti_aliasing_cb->isChecked());
      _settings->setValue("fullscreen", ui->_fullscreen_cb->isChecked());
      _settings->setValue("background_fps_limit", ui->_background_fps_limit_cb->isChecked());
      _settings->setValue("directional_lightning", ui->_directional_light_cb->isChecked());
      _settings->setValue("unload_dist", ui->_adt_unload_dist->value());
      _settings->setValue("unload_interval", ui->_adt_unload_check_interval->value());
      _settings->setValue("loading_radius", ui->_adt_loading_radius->value());
      _settings->setValue("uid_startup_check", ui->_uid_cb->isChecked());
      _settings->setValue("auto_load_fav_project", ui->_load_fav_cb->isChecked());
      _settings->setValue("additional_file_loading_log", ui->_additional_file_loading_log->isChecked());
      _settings->setValue("keyboard_locale", ui->_keyboard_locale->currentText());
      _settings->setValue("systemWindowFrame", ui->_systemWindowFrame->isChecked());
      _settings->setValue("nativeMenubar", ui->_nativeMenubar->isChecked());
      _settings->setValue("classicUI", ui->_classic_ui->isChecked());
      _settings->setValue("use_mclq_liquids_export", ui->_use_mclq_liquids_export->isChecked());

#ifdef USE_MYSQL_UID_STORAGE
      _settings->setValue ("project/mysql/enabled", ui->MySQL_box->isChecked());
      _settings->setValue ("project/mysql/server", ui->_mysql_server_field->text());
      _settings->setValue ("project/mysql/user", ui->_mysql_user_field->text());
      _settings->setValue ("project/mysql/pwd", ui->_mysql_pwd_field->text());
      _settings->setValue ("project/mysql/db", ui->_mysql_db_field->text());
      _settings->setValue ("project/mysql/port", ui->_mysql_port_field->text());
#endif

      _settings->setValue("wireframe/type", ui->radio_wire_cursor->isChecked());
      _settings->setValue("wireframe/radius", ui->_wireframe_radius->value());
      _settings->setValue("wireframe/width", ui->_wireframe_width->value());
      _settings->setValue("wireframe/color", ui->_wireframe_color->color());
      _settings->setValue("theme", ui->_theme->currentText());
      _settings->setValue("assetBrowser/background_color", ui->assetBrowserBgCol->color());
      _settings->setValue("assetBrowser/diffuse_light", ui->assetBrowserDiffuseLight->color());
      _settings->setValue("assetBrowser/ambient_light", ui->assetBrowserAmbientLight->color());
      _settings->setValue("assetBrowser/copy_to_clipboard", ui->assetBrowserCopyToClipboard->isChecked());
      _settings->setValue("assetBrowser/default_model", ui->assetBrowserDefaultModel->text());
      _settings->setValue("assetBrowser/move_sensitivity", ui->assetBrowserMoveSensitivity->value());
      _settings->setValue("assetBrowser/render_asset_preview", ui->assetBrowserRenderAssetPreview->isChecked());
      _settings->setValue("fps_limit", ui->_fps_limit_slider->value());

      _settings->sync();

      // calls MapView::onSettingsSave()
      emit saved();
    }
  }
}
