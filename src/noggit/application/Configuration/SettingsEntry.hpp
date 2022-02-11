// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_SETTINGSENTRY_HPP
#define NOGGIT_SETTINGSENTRY_HPP

#include <type_traits>
#include <QObject>
#include <QColor>

#include <variant>

class QJsonObject;
class QWidget;

namespace Noggit::Application::Configuration
{

  using filepath = std::string;

  enum class SettingsEntryType
  {
    FILE_PATH,
    DIR_PATH,
    STRING,
    COLOR_RGB,
    COLOR_RGBA,
    DOUBLE,
    INT,
    BOOL
  };

  enum class SettingsEntryDataType
  {
    STRING,
    DOUBLE,
    COLOR,
    INT,
    BOOL
  };


  class SettingsEntry : public QObject
  {
    Q_OBJECT
  public:
    SettingsEntry() = default;

    virtual QWidget* createWidget(QWidget* parent) = 0;
    virtual QJsonObject valueToJson() = 0;
    virtual void valueFromJson(QJsonObject const& json_obj) = 0;

    //ValueType value() { return _value; };
    //void setValue(ValueType value) { _value = value; emit valueChanged(_type, &_value); }

  signals:
    //void valueChanged(std::variant< const& value);

  private:
    SettingsEntryType _type;
    std::variant<filepath, std::string> _value;

  };




}

#endif //NOGGIT_SETTINGSENTRY_HPP
