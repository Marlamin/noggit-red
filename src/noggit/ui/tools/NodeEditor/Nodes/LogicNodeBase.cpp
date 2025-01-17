// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicNodeBase.hpp"

#include <external/NodeEditor/include/nodes/Node>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>


namespace Noggit
{
  namespace Ui::Tools::NodeEditor::Nodes
  {
    LogicNodeBase::LogicNodeBase()
    {
    }

    void LogicNodeBase::resetInterationIndex()
    {
      _iteration_index = 0;
    }

    unsigned LogicNodeBase::getIterationindex() const
    {
      return _iteration_index;
    }

    bool LogicNodeBase::isLogicNode()
    {
      return true;
    }

    bool LogicNodeBase::isIterable() const
    {
      return _is_iterable;
    }

    void LogicNodeBase::setNIterations(unsigned n_iterations)
    {
      _n_iterations = n_iterations;
    }

    unsigned LogicNodeBase::getNIteraitons() const
    {
      return _n_iterations;
    }

    void LogicNodeBase::setIterationIndex(unsigned index)
    {
      _iteration_index = index;
    }

    NodeValidationState LogicNodeBase::validate()
    {
      setValidationState(NodeValidationState::Valid);
      auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

      if (!logic)
      {
        setValidationState(NodeValidationState::Error);
        setValidationMessage("Error: Failed to evaluate logic input");

        _out_ports[0].out_value = std::make_shared<LogicData>(false);
        _node->onDataUpdated(0);
      }

      return _validation_state;
    }

    void LogicNodeBase::setIsIterable(bool is_iterable)
    {
      _is_iterable = is_iterable;
    }
  }
}
