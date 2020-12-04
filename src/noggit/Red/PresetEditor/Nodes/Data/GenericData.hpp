#ifndef NOGGIT_GENERICDATA_HPP
#define NOGGIT_GENERICDATA_HPP

#include <external/NodeEditor/include/nodes/NodeDataModel>
#include <external/glm/glm.hpp>

#include <type_traits>
#include <QString>
#include <functional>

using QtNodes::NodeDataType;
using QtNodes::NodeData;


template<typename Ty, const char* type_id, const char* type_name, typename C>
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


private:
    Ty _value;
};

template<typename T>
struct toQStringGeneric
{
  static QString to_string(T const& value) { QString(std::to_string(value).c_str()); }
};

#define DECLARE_NODE_DATA_TYPE(TYPE_ID, TYPE_NAME, UNDERLYING_TYPE)                       \
  constexpr char  TYPE_ID##_typeid[] = #TYPE_ID;                                          \
  constexpr char  TYPE_ID##_typename[] = #TYPE_NAME;                                      \
  using TYPE_NAME##Data = GenericData<UNDERLYING_TYPE,                                    \
  TYPE_ID##_typeid, TYPE_ID##_typename, toQStringGeneric<UNDERLYING_TYPE>>;

DECLARE_NODE_DATA_TYPE(int, Integer, int)
DECLARE_NODE_DATA_TYPE(double, Decimal, double)
DECLARE_NODE_DATA_TYPE(bool, Boolean, bool)
DECLARE_NODE_DATA_TYPE(string, String, std::string)
DECLARE_NODE_DATA_TYPE(logic, Logic, bool)

// GLM types
DECLARE_NODE_DATA_TYPE(vec2, Vector2D, glm::vec2)
DECLARE_NODE_DATA_TYPE(vec3, Vector3D, glm::vec3)
DECLARE_NODE_DATA_TYPE(vec4, Vector4D, glm::vec4)
DECLARE_NODE_DATA_TYPE(mat4, Matrix4x4, glm::mat4)
DECLARE_NODE_DATA_TYPE(mat3, Matrix3x3, glm::mat3)
DECLARE_NODE_DATA_TYPE(quat, Quaternion, glm::quat)



#endif //NOGGIT_GENERICDATA_HPP
