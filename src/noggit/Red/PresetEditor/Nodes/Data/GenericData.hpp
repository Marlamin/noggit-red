#ifndef NOGGIT_GENERICDATA_HPP
#define NOGGIT_GENERICDATA_HPP

#include <external/NodeEditor/include/nodes/NodeDataModel>
#include <external/glm/glm.hpp>

#include <type_traits>
#include <QString>
#include <functional>
#include <limits>
#include <cstdint>

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>

using QtNodes::NodeDataType;
using QtNodes::NodeData;


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
      return NodeDataType {type_id, type_name};
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
};

struct DefaultDecimalWidget
{
    static QWidget* generate(QWidget* parent)
    {
      auto widget = new QDoubleSpinBox(parent);
      widget->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
      return widget;
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
};

struct DefaultStringWidget
{
    static QWidget* generate(QWidget* parent) { return new QLineEdit(parent); }
};

struct NoDefaultWidget
{
    static QWidget* generate(QWidget* parent) { return new QLabel("", parent); }
};


#define DECLARE_NODE_DATA_TYPE(TYPE_ID, TYPE_NAME, UNDERLYING_TYPE, DEFAULT_WIDGET_GEN)   \
  constexpr char  TYPE_ID##_typeid[] = #TYPE_ID;                                          \
  constexpr char  TYPE_ID##_typename[] = #TYPE_NAME;                                      \
  using TYPE_NAME##Data = GenericData<UNDERLYING_TYPE,                                    \
  TYPE_ID##_typeid, TYPE_ID##_typename, toQStringGeneric<UNDERLYING_TYPE>, DEFAULT_WIDGET_GEN>;

DECLARE_NODE_DATA_TYPE(int, Integer, int, DefaultIntWidget)
DECLARE_NODE_DATA_TYPE(double, Decimal, double, DefaultDecimalWidget)
DECLARE_NODE_DATA_TYPE(bool, Boolean, bool, DefaultBooleanWidget)
DECLARE_NODE_DATA_TYPE(string, String, std::string, DefaultStringWidget)
DECLARE_NODE_DATA_TYPE(logic, Logic, bool, NoDefaultWidget)

// GLM types
DECLARE_NODE_DATA_TYPE(vec2, Vector2D, glm::vec2, NoDefaultWidget)
DECLARE_NODE_DATA_TYPE(vec3, Vector3D, glm::vec3, NoDefaultWidget)
DECLARE_NODE_DATA_TYPE(vec4, Vector4D, glm::vec4, NoDefaultWidget)
DECLARE_NODE_DATA_TYPE(mat4, Matrix4x4, glm::mat4, NoDefaultWidget)
DECLARE_NODE_DATA_TYPE(mat3, Matrix3x3, glm::mat3, NoDefaultWidget)
DECLARE_NODE_DATA_TYPE(quat, Quaternion, glm::quat, NoDefaultWidget)

DECLARE_NODE_DATA_TYPE(any, Any, std::nullptr_t, NoDefaultWidget);



#endif //NOGGIT_GENERICDATA_HPP
