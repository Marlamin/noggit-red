// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEGENERATORBASE_HPP
#define NOGGIT_NOISEGENERATORBASE_HPP

#include <noggit/Red/NodeEditor/Nodes/BaseNode.hpp>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace Red::NodeEditor::Nodes
    {
        class NoiseGeneratorBase : public BaseNode
        {
        Q_OBJECT

        public:
            NoiseGeneratorBase();
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        protected:
            template<typename T>
            bool checkBounds(T variable, T min, T max, std::string const& name)
            {
              if (variable < min || variable > max)
              {
                setValidationState(NodeValidationState::Error);
                setValidationMessage(("Error: input \"" + name + "\" is out of range.").c_str());
                return false;
              }

              return true;
            }

        };

    }

}

#endif //NOGGIT_NOISEGENERATORBASE_HPP
