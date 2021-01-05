#ifndef NOGGIT_GENERICTYPECONVERTER_HPP
#define NOGGIT_GENERICTYPECONVERTER_HPP

#include <external/NodeEditor/include/nodes/NodeDataModel>
#include <boost/format.hpp>
#include "GenericData.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;

template <typename T_from, typename T_to, typename C>
class GenericTypeConverter
{

public:

    std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data)
    {
      auto data_ptr = static_cast<T_from*>(data.get());

      if (data_ptr)
      {
        _data = std::make_shared<T_to>(C::convert(data_ptr->value()));
      }
      else
      {
        _data.reset();
      }

      return _data;
    };

private:

    std::shared_ptr<NodeData> _data;
};

template<typename T_from, typename T_to>
struct UnderlyingTypeConvertGeneric
{
  static T_to convert(T_from const& value)
  {
    return static_cast<T_to>(value);
  }
};

template<typename T_from>
struct StringConverter
{
    static std::string convert(T_from const& value)
    {
      return std::to_string(value);
    }
};

template<typename T_from>
struct BasicDataConverter
{
    static nullptr_t convert(T_from const& value) { return nullptr; }
};

// Color converters

template<typename T_from>
struct ColorIntegerConverter
{
    static QColor convert(T_from const& value) { return QColor::fromRgb(value, value, value, value); }
};


struct ColorDecimalConverter
{
    static QColor convert(double const& value) { return QColor::fromRgbF(value, value, value, value); }
};

struct ColorStringConverter
{
    static std::string convert(QColor const& value) { return (boost::format("Color<%d, %d, %d, %d>") % value.red() % value.green() % value.blue() % value.alpha()).str(); }
};

struct ColorVector4DConverter
{
    static glm::vec4 convert(QColor const& value) { return glm::vec4(value.redF(), value.greenF(), value.blueF(), value.alphaF()); };
};



#define DECLARE_TYPE_CONVERTER(DATA_FROM, DATA_TO, UTYPE_FROM, UTYPE_TO)  \
  using DATA_FROM##To##DATA_TO##TypeConverter =                           \
  GenericTypeConverter<DATA_FROM##Data, DATA_TO##Data,                    \
  UnderlyingTypeConvertGeneric<UTYPE_FROM, UTYPE_TO>>;


#define DECLARE_TYPE_CONVERTER_EXT(DATA_FROM, DATA_TO, CONVERT_FUNCTOR)  \
  using DATA_FROM##To##DATA_TO##TypeConverter =                                                \
  GenericTypeConverter<DATA_FROM##Data, DATA_TO##Data, CONVERT_FUNCTOR>;


DECLARE_TYPE_CONVERTER(Decimal, Integer, double, int)
DECLARE_TYPE_CONVERTER(Decimal, UnsignedInteger, double, unsigned int)
DECLARE_TYPE_CONVERTER(Decimal, Boolean, double, bool)

DECLARE_TYPE_CONVERTER(Integer, Decimal, int, double)
DECLARE_TYPE_CONVERTER(Integer, UnsignedInteger, int, unsigned int)
DECLARE_TYPE_CONVERTER(Integer, Boolean, int, bool)

DECLARE_TYPE_CONVERTER(Boolean, Integer, bool, int)
DECLARE_TYPE_CONVERTER(Boolean, UnsignedInteger, bool, unsigned int)
DECLARE_TYPE_CONVERTER(Boolean, Decimal, bool, double)

DECLARE_TYPE_CONVERTER(UnsignedInteger, Integer, unsigned int, int)
DECLARE_TYPE_CONVERTER(UnsignedInteger, Decimal, unsigned int, double)
DECLARE_TYPE_CONVERTER(UnsignedInteger, Boolean, unsigned int, bool)

DECLARE_TYPE_CONVERTER_EXT(Decimal, String, StringConverter<double>)
DECLARE_TYPE_CONVERTER_EXT(Integer, String, StringConverter<int>)
DECLARE_TYPE_CONVERTER_EXT(UnsignedInteger, String, StringConverter<unsigned int>)

// Polymorph types
DECLARE_TYPE_CONVERTER_EXT(Integer, Basic, BasicDataConverter<int>)
DECLARE_TYPE_CONVERTER_EXT(UnsignedInteger, Basic, BasicDataConverter<unsigned int>)
DECLARE_TYPE_CONVERTER_EXT(Decimal, Basic, BasicDataConverter<double>)
DECLARE_TYPE_CONVERTER_EXT(String, Basic, BasicDataConverter<std::string>)

// Custom types
DECLARE_TYPE_CONVERTER_EXT(Integer, Color, ColorIntegerConverter<int>)
DECLARE_TYPE_CONVERTER_EXT(UnsignedInteger, Color, ColorIntegerConverter<unsigned int>)
DECLARE_TYPE_CONVERTER_EXT(Decimal, Color, ColorIntegerConverter<double>)
DECLARE_TYPE_CONVERTER_EXT(Color, String, ColorStringConverter)
DECLARE_TYPE_CONVERTER_EXT(Color, Vector4D, ColorVector4DConverter)


#endif //NOGGIT_GENERICTYPECONVERTER_HPP
