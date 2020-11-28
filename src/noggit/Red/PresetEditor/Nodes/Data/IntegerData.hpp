#ifndef NOGGIT_INTEGERDATA_HPP
#define NOGGIT_INTEGERDATA_HPP

#include <external/NodeEditor/include/nodes/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;


class IntegerData : public NodeData
{
public:

    IntegerData()
        : _number(0.0)
    {}

    IntegerData(int const number)
        : _number(number)
    {}

    NodeDataType type() const override
    {
      return NodeDataType {"integer",
                           "Integer"};
    }

    int number() const
    { return _number; }

    QString numberAsText() const
    { return QString::number(_number); }

private:

    int _number;
};


#endif //NOGGIT_INTEGERDATA_HPP
