#ifndef NOGGIT_GENERICDATA_HPP
#define NOGGIT_GENERICDATA_HPP

#include "noggit/Red/NodeEditor/Nodes/Widgets/ProcedureSelector.hpp"

#include <external/NodeEditor/include/nodes/NodeDataModel>
#include <external/glm/glm.hpp>
#include <external/glm/gtc/quaternion.hpp>
#include <external/qt-color-widgets/qt-color-widgets/color_selector.hpp>

#include <noggit/ui/font_awesome.hpp>

#include <type_traits>
#include <QString>
#include <functional>
#include <limits>
#include <cstdint>
#include <unordered_map>

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
#include <QPixmap>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

using namespace noggit::Red::NodeEditor::Nodes;

template<typename Ty, const char* type_id, const char* type_name, typename C, typename D>
class GenericData : public NodeData
{
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

    [[nodiscard]]
    QString toQString() const { return C::to_string(_value); }

    std::unique_ptr<NodeData> instantiate() override
    {
      return std::make_unique<GenericData<Ty, type_id, type_name, C, D>>();
    }

    QWidget* default_widget(QWidget* parent) override
    {
      return D::generate(parent);
    }

    std::shared_ptr<NodeData> default_widget_data(QWidget* widget) override
    {
      auto value = D::value(widget);

      if constexpr (std::is_same<typeof(value), nullptr_t>::value)
      {
        auto data_ptr = std::make_shared<GenericData<Ty, type_id, type_name, C, D>>();
        data_ptr.reset();
        return data_ptr;
      }
      else
      {
        return std::make_shared<GenericData<Ty, type_id, type_name, C, D>>(value);
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
  static QString to_string(T const& value) { QString(std::to_string(value).c_str()); }
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

struct DefaultDecimalWidget
{
    static QWidget* generate(QWidget* parent)
    {
      auto widget = new QDoubleSpinBox(parent);
      widget->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
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
        axis->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
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
      widget->showDialog();
      widget->closeDialog();

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

    static nullptr_t value(QWidget* widget)
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

struct TypeFactory
{
    static NodeData* create(const std::string& id)
    {
      const Creators_t::const_iterator iter = static_creators().find(id);
      return iter == static_creators().end() ? 0 : (*iter->second)();
    }

private:
    typedef NodeData* Creator_t();
    typedef std::map<std::string, Creator_t*> Creators_t;
    static Creators_t& static_creators() { static Creators_t s_creators; return s_creators; }

    template<typename T> struct Register
    {
        static NodeData* create() { return new T(); };
        static Creator_t* init_creator(const std::string& id) { return static_creators()[id] = create; }
        static Creator_t* creator;
    };
};

#define DECLARE_NODE_DATA_TYPE(TYPE_ID, TYPE_NAME, UNDERLYING_TYPE, DEFAULT_WIDGET_GEN)   \
  constexpr char  TYPE_ID##_typeid[] = #TYPE_ID;                                          \
  constexpr char  TYPE_ID##_typename[] = #TYPE_NAME;                                      \
  using TYPE_NAME##Data = GenericData<UNDERLYING_TYPE,                                    \
  TYPE_ID##_typeid, TYPE_ID##_typename, toQStringGeneric<UNDERLYING_TYPE>, DEFAULT_WIDGET_GEN>; \
                                                                                          \
  template<> TypeFactory::Creator_t* TypeFactory::Register<TYPE_NAME##Data>::creator = TypeFactory::Register<TYPE_NAME##Data>::init_creator(#TYPE_ID); \

DECLARE_NODE_DATA_TYPE(logic, Logic, bool, NoDefaultWidget)

DECLARE_NODE_DATA_TYPE(int, Integer, int, DefaultIntWidget)
DECLARE_NODE_DATA_TYPE(uint, UnsignedInteger, unsigned int, DefaultIntWidget)

DECLARE_NODE_DATA_TYPE(double, Decimal, double, DefaultDecimalWidget)
DECLARE_NODE_DATA_TYPE(bool, Boolean, bool, DefaultBooleanWidget)
DECLARE_NODE_DATA_TYPE(string, String, std::string, DefaultStringWidget)

// GLM types
DECLARE_NODE_DATA_TYPE(vec2, Vector2D, glm::vec2, DefaultVector2DWidget)
DECLARE_NODE_DATA_TYPE(vec3, Vector3D, glm::vec3, DefaultVector3DWidget)
DECLARE_NODE_DATA_TYPE(vec4, Vector4D, glm::vec4, DefaultVector4DWidget)
DECLARE_NODE_DATA_TYPE(mat4, Matrix4x4, glm::mat4, NoDefaultWidget)
DECLARE_NODE_DATA_TYPE(quat, Quaternion, glm::quat, NoDefaultWidget)

// Polymorph types
DECLARE_NODE_DATA_TYPE(any, Any, std::nullptr_t, NoDefaultWidget);
DECLARE_NODE_DATA_TYPE(basic, Basic, std::nullptr_t, NoDefaultWidget);
DECLARE_NODE_DATA_TYPE(undefined, Undefined, std::nullptr_t, NoDefaultWidget);
DECLARE_NODE_DATA_TYPE(procedure, Procedure, std::string, DefaultProcedureWidget);
DECLARE_NODE_DATA_TYPE(list, List, std::vector<std::shared_ptr<NodeData>>*, NoDefaultWidget);

// Custom types
DECLARE_NODE_DATA_TYPE(color, Color, glm::vec4, DefaultColorWidget);
DECLARE_NODE_DATA_TYPE(image, Image, QPixmap, NoDefaultWidget);



#endif //NOGGIT_GENERICDATA_HPP
