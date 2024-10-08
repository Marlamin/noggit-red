#ifndef NOGGIT_GENERICDATA_HPP
#define NOGGIT_GENERICDATA_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/Widgets/ProcedureSelector.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Widgets/QUnsignedSpinBox.hpp>

#include <external/NodeEditor/include/nodes/NodeDataModel>
#include <external/glm/glm.hpp>
#include <external/glm/gtc/quaternion.hpp>
#include <external/glm/gtx/string_cast.hpp>
#include <external/qt-color-widgets/qt-color-widgets/color_selector.hpp>
#include <external/libnoise/src/noise/noise.h>

#include <noggit/ui/FontAwesome.hpp>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/SceneObject.hpp>

#include <type_traits>
#include <QString>
#include <functional>
#include <limits>
#include <cstdint>
#include <unordered_map>
#include <array>
#include <string_view>
#include <algorithm>

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QColor>
#include <QImage>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;
using namespace std::literals;

template<typename Ty, const char* type_id, const char* type_name, typename C, typename D>
class GenericData : public NodeData
{
    typedef GenericData<Ty, type_id, type_name, C, D> ThisType;

    /* bypassing indirection overhead: passing by value if Ty fits into a machine word, passing by reference otherwise */
    typedef std::conditional_t<sizeof(Ty) <= sizeof(std::size_t), Ty, Ty const&> Type;
public:
    GenericData()
        : _value{} { }

    GenericData(Type value)
        : _value{value} { }

    [[nodiscard]]
    NodeDataType type() const override
    {
      return NodeDataType {type_id, type_name, _parameter_type_id};
    }

    [[nodiscard]]
    Type value() const { return _value; }

    Ty* value_ptr() { return &_value; };

    [[nodiscard]]
    QString toQString() const override { return C::to_string(_value); }

    std::unique_ptr<NodeData> instantiate() override
    {
      return std::make_unique<ThisType>();
    }

    QWidget* default_widget(QWidget* parent) override
    {
      QWidget* widget = D::generate(parent);
      widget->setAttribute(Qt::WA_NoSystemBackground);
      return widget;
    }

    std::shared_ptr<NodeData> default_widget_data(QWidget* widget) override
    {
      auto value = D::value(widget);

      if constexpr (std::is_same<decltype(value), std::nullptr_t>::value)
      {
        auto data_ptr = std::make_shared<ThisType>();
        data_ptr.reset();
        return data_ptr;
      }
      else
      {
        return std::make_shared<ThisType>(value);
      }

    }

    void to_json(QWidget* widget, QJsonObject& json_obj, const std::string& name) override
    {
      D::toJson(widget, json_obj, name);
    };

    void from_json(QWidget* widget, const QJsonObject& json_obj, const std::string& name) override
    {
      D::fromJson(widget, json_obj, name);
    };

private:
    Ty _value;
};

template<typename T>
struct toQStringGeneric
{
  static QString to_string(T const& value) { return QString(std::to_string(value).c_str()); }
};

template<typename T>
struct toQStringString
{
    static QString to_string(T const& value) { return QString(value.c_str()); }
};

template<typename T>
struct toQStringGlm
{
    static QString to_string(T const& value) { return QString(glm::to_string(value).c_str()); }
};

template<typename T>
struct toQStringNA
{
    static QString to_string(T const& value) { return QString("Passed"); }
};

struct DefaultIntWidget
{
    static QWidget* generate(QWidget* parent)
    {
      auto widget = new QSpinBox(parent);
      widget->setRange(std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::max());
      return widget;
    }

    static int value(QWidget* widget)
    {
      return static_cast<QSpinBox*>(widget)->value();
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
      static_cast<QSpinBox*>(widget)->setValue(value.toInt());
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
      json_obj[name.c_str()] = static_cast<QSpinBox*>(widget)->value();
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
      int value = json_obj[name.c_str()].toInt();
      static_cast<QSpinBox*>(widget)->setValue(value);
    }
};

struct DefaultUnsignedIntWidget
{
    static QWidget* generate(QWidget* parent)
    {
      auto widget = new QUnsignedSpinBox(parent);
      widget->setRange(0, std::numeric_limits<quint32>::max());
      return widget;
    }

    static unsigned int value(QWidget* widget)
    {
      return static_cast<QUnsignedSpinBox*>(widget)->value();
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
      static_cast<QUnsignedSpinBox*>(widget)->setValue(value.toString().toUInt());
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
      json_obj[name.c_str()] = QString::number(static_cast<QUnsignedSpinBox*>(widget)->value());
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
      quint32 value = json_obj[name.c_str()].toString().toUInt();
      static_cast<QUnsignedSpinBox*>(widget)->setValue(value);
    }
};

struct DefaultDecimalWidget
{
    static QWidget* generate(QWidget* parent)
    {
      auto widget = new QDoubleSpinBox(parent);
      widget->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
      widget->setDecimals(7);
      return widget;
    }

    static double value(QWidget* widget)
    {
      return static_cast<QDoubleSpinBox*>(widget)->value();
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
      static_cast<QDoubleSpinBox*>(widget)->setValue(value.toDouble());
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
      json_obj[name.c_str()] = static_cast<QDoubleSpinBox*>(widget)->value();
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
      double value = json_obj[name.c_str()].toDouble();
      static_cast<QDoubleSpinBox*>(widget)->setValue(value);
    }
};

struct DefaultBooleanWidget
{
    static QWidget* generate(QWidget* parent)
    {
      auto widget = new QCheckBox(parent);
      widget->setChecked(false);
      return widget;
    }

    static bool value(QWidget* widget)
    {
      return static_cast<QCheckBox*>(widget)->isChecked();
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
      static_cast<QCheckBox*>(widget)->setChecked(value.toBool());
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
      json_obj[name.c_str()] = static_cast<QCheckBox*>(widget)->isChecked();
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
      bool value = json_obj[name.c_str()].toBool();
      static_cast<QCheckBox*>(widget)->setChecked(value);
    }
};

struct DefaultStringWidget
{
    static QWidget* generate(QWidget* parent) { return new QLineEdit(parent); }

    static std::string value(QWidget* widget)
    {
      return static_cast<QLineEdit*>(widget)->text().toStdString();
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
      static_cast<QLineEdit*>(widget)->setText(value.toString());
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
      json_obj[name.c_str()] = static_cast<QLineEdit*>(widget)->text();
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
      QString value = json_obj[name.c_str()].toString();
      static_cast<QLineEdit*>(widget)->setText(value);
    }
};

struct DefaultProcedureWidget
{
    static QWidget* generate(QWidget* parent)
    {
      return new ProcedureSelector(parent);
    }

    static std::string value(QWidget* widget)
    {
      return static_cast<QPushButton*>(widget->layout()->itemAt(0)->widget())->text().toStdString();
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
      static_cast<QPushButton*>(widget->layout()->itemAt(0)->widget())->setText(value.toString());
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
      json_obj[name.c_str()] = static_cast<QPushButton*>(widget->layout()->itemAt(0)->widget())->text();
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
      QString value = json_obj[name.c_str()].toString();
      static_cast<QPushButton*>(widget->layout()->itemAt(0)->widget())->setText(value);
    }
};

template <typename T, int SIZE>
struct DefaultVectorWidget
{
    static QWidget* generate(QWidget* parent)
    {
      auto widget = new QWidget(parent);
      auto layout = new QVBoxLayout(widget);

      for (int i = 0; i < SIZE; ++i)
      {
        auto axis = new QDoubleSpinBox(widget);
        axis->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
        axis->setDecimals(7);
        layout->addWidget(axis);
      }

      return widget;
    }

    static T value(QWidget* widget)
    {
      T vector;

      for (int i = 0; i < SIZE; ++i)
      {
        vector[i] = static_cast<QDoubleSpinBox*>(widget->layout()->itemAt(i)->widget())->value();
      }

      return vector;
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
      auto array = value.toArray();

      for (int i = 0; i < SIZE; ++i)
      {
        static_cast<QDoubleSpinBox*>(widget->layout()->itemAt(i)->widget())->setValue(array[i].toDouble());
      }
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
      QJsonArray array = QJsonArray();

      for (int i = 0; i < SIZE; ++i)
      {
        array.push_back(static_cast<QDoubleSpinBox*>(widget->layout()->itemAt(i)->widget())->value());
      }

      json_obj[name.c_str()] = array;
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
      QJsonArray array = json_obj[name.c_str()].toArray();

      for (int i = 0; i < SIZE; ++i)
      {
        static_cast<QDoubleSpinBox*>(widget->layout()->itemAt(i)->widget())->setValue(array[i].toDouble());
      }
    }
};

using DefaultVector2DWidget = DefaultVectorWidget<glm::vec2, 2>;
using DefaultVector3DWidget = DefaultVectorWidget<glm::vec3, 3>;
using DefaultVector4DWidget = DefaultVectorWidget<glm::vec4, 4>;

struct DefaultColorWidget
{
    static QWidget* generate(QWidget* parent)
    {
      auto widget = new color_widgets::ColorSelector(parent);
      widget->setColor({255, 255, 255, 255});
      widget->setDisplayMode(color_widgets::ColorPreview::SplitAlpha);

      // TODO: figure out what goes wrong here without this
      //widget->showDialog();
      // widget->closeDialog();

      return widget;
    }

    static glm::vec4 value(QWidget* widget)
    {
      QColor color = static_cast<color_widgets::ColorSelector*>(widget)->color();
      return glm::vec4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
      auto array = value.toArray();
      QColor color = QColor::fromRgbF(array[0].toDouble(), array[1].toDouble(), array[2].toDouble(), array[3].toDouble());
      static_cast<color_widgets::ColorSelector*>(widget)->setColor(color);
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
      QJsonArray array = QJsonArray();
      QColor color = static_cast<color_widgets::ColorSelector*>(widget)->color();

      array.push_back(color.redF());
      array.push_back(color.greenF());
      array.push_back(color.blueF());
      array.push_back(color.alphaF());

      json_obj[name.c_str()] = array;
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
      QJsonArray array = json_obj[name.c_str()].toArray();
      QColor color = QColor::fromRgbF(array[0].toDouble(), array[1].toDouble(), array[2].toDouble(), array[3].toDouble());
      static_cast<color_widgets::ColorSelector*>(widget)->setColor(color);
    }
};

struct NoDefaultWidget
{
    static QWidget* generate(QWidget* parent) { return new QLabel("", parent); }

    static std::nullptr_t value(QWidget* widget)
    {
      return nullptr;
    }

    static void setValue(QWidget* widget, QJsonValue& value)
    {
    }

    static void toJson(QWidget* widget, QJsonObject& json_obj, const std::string& name)
    {
    }

    static void fromJson(QWidget* widget, const QJsonObject& json_obj, const std::string& name)
    {
    }

};

#define DECLARE_NODE_DATA_TYPE(TYPE_ID, TYPE_NAME, UNDERLYING_TYPE, DEFAULT_WIDGET_GEN)   \
  inline constexpr char  TYPE_ID##_typeid[] = #TYPE_ID;                                   \
  inline constexpr char  TYPE_ID##_typename[] = #TYPE_NAME;                               \
  using TYPE_NAME##Data = GenericData<UNDERLYING_TYPE,                                    \
  TYPE_ID##_typeid, TYPE_ID##_typename, toQStringGeneric<UNDERLYING_TYPE>, DEFAULT_WIDGET_GEN>;


#define DECLARE_NODE_DATA_TYPE_EXT(TYPE_ID, TYPE_NAME, UNDERLYING_TYPE, DEFAULT_WIDGET_GEN, STRING_C)   \
  inline constexpr char  TYPE_ID##_typeid[] = #TYPE_ID;                                                 \
  inline constexpr char  TYPE_ID##_typename[] = #TYPE_NAME;                                             \
  using TYPE_NAME##Data = GenericData<UNDERLYING_TYPE,                                                  \
  TYPE_ID##_typeid, TYPE_ID##_typename, STRING_C <UNDERLYING_TYPE>, DEFAULT_WIDGET_GEN>;


DECLARE_NODE_DATA_TYPE(logic, Logic, bool, NoDefaultWidget)

DECLARE_NODE_DATA_TYPE(int, Integer, int, DefaultIntWidget)
DECLARE_NODE_DATA_TYPE(uint, UnsignedInteger, unsigned int, DefaultUnsignedIntWidget)
DECLARE_NODE_DATA_TYPE(double, Decimal, double, DefaultDecimalWidget)
DECLARE_NODE_DATA_TYPE(bool, Boolean, bool, DefaultBooleanWidget)
DECLARE_NODE_DATA_TYPE_EXT(string, String, std::string, DefaultStringWidget, toQStringString)

// GLM types
DECLARE_NODE_DATA_TYPE_EXT(vec2, Vector2D, glm::vec2, DefaultVector2DWidget, toQStringGlm)
DECLARE_NODE_DATA_TYPE_EXT(vec3, Vector3D, glm::vec3, DefaultVector3DWidget, toQStringGlm)
DECLARE_NODE_DATA_TYPE_EXT(vec4, Vector4D, glm::vec4, DefaultVector4DWidget, toQStringGlm)
DECLARE_NODE_DATA_TYPE_EXT(mat4, Matrix4x4, glm::mat4, NoDefaultWidget, toQStringGlm)
DECLARE_NODE_DATA_TYPE_EXT(quat, Quaternion, glm::quat, NoDefaultWidget, toQStringGlm)

// Polymorph types
DECLARE_NODE_DATA_TYPE_EXT(any, Any, std::nullptr_t, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(basic, Basic, std::nullptr_t, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(undefined, Undefined, std::nullptr_t, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(procedure, Procedure, std::string, DefaultProcedureWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(list, List, std::vector<std::shared_ptr<NodeData>>*, NoDefaultWidget, toQStringNA);

// Custom types
DECLARE_NODE_DATA_TYPE_EXT(color, Color, glm::vec4, DefaultColorWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(image, Image, QImage, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(noise, Noise, noise::module::Module*, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(chunk, Chunk, MapChunk*, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(tile, Tile, MapTile*, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(object, ObjectInstance, SceneObject*, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(json, JSON, QJsonObject, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(json_array, JSONArray, QJsonArray, NoDefaultWidget, toQStringNA);
DECLARE_NODE_DATA_TYPE_EXT(json_value, JSONValue, QJsonValue, NoDefaultWidget, toQStringNA);

#define CREATE_DATA_PAIR(TYPE_ID, TYPE_NAME) \
std::make_pair(#TYPE_ID ##sv, &_create<TYPE_NAME##Data>)

struct TypeFactory
{

    static NodeData* create(const std::string& id)
    {

      auto lambda = [&id](std::pair<std::string_view, NodeData*(*)()> const& pair) -> bool
      {
        return pair.first == id;
      };

      assert(std::find_if(_creators_map.begin(), _creators_map.end(), lambda) != _creators_map.end());

      return std::find_if(_creators_map.begin(), _creators_map.end(), lambda)->second();
    }

private:

    template <typename Ty>
    static NodeData* _create()
    {
      return new Ty();
    }

    static constexpr std::array<std::pair<std::string_view, NodeData*(*)()>, 25> _creators_map = {
                                                                                              CREATE_DATA_PAIR(logic, Logic),
                                                                                              CREATE_DATA_PAIR(int, Integer),
                                                                                              CREATE_DATA_PAIR(uint, UnsignedInteger),
                                                                                              CREATE_DATA_PAIR(double, Decimal),
                                                                                              CREATE_DATA_PAIR(bool, Boolean),
                                                                                              CREATE_DATA_PAIR(string, String),
                                                                                              CREATE_DATA_PAIR(vec2, Vector2D),
                                                                                              CREATE_DATA_PAIR(vec3, Vector3D),
                                                                                              CREATE_DATA_PAIR(vec4, Vector4D),
                                                                                              CREATE_DATA_PAIR(mat4, Matrix4x4),
                                                                                              CREATE_DATA_PAIR(quat, Quaternion),
                                                                                              CREATE_DATA_PAIR(any, Any),
                                                                                              CREATE_DATA_PAIR(basic, Basic),
                                                                                              CREATE_DATA_PAIR(undefined, Undefined),
                                                                                              CREATE_DATA_PAIR(procedure, Procedure),
                                                                                              CREATE_DATA_PAIR(list, List),
                                                                                              CREATE_DATA_PAIR(color, Color),
                                                                                              CREATE_DATA_PAIR(image, Image),
                                                                                              CREATE_DATA_PAIR(noise, Noise),
                                                                                              CREATE_DATA_PAIR(chunk, Chunk),
                                                                                              CREATE_DATA_PAIR(tile, Tile),
                                                                                              CREATE_DATA_PAIR(object, ObjectInstance),
                                                                                              CREATE_DATA_PAIR(json, JSON),
                                                                                              CREATE_DATA_PAIR(json_array, JSONArray),
                                                                                              CREATE_DATA_PAIR(json_value, JSONValue)
                                                                                             };

};

#endif //NOGGIT_GENERICDATA_HPP
