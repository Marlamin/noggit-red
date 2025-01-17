#ifndef NOGGIT_BASENODE_HPP
#define NOGGIT_BASENODE_HPP

#include <external/NodeEditor/include/nodes/NodeDataModel>

#include <vector>
#include <memory>

namespace QtNodes
{
  class Connection;
  class NodeData;
}

class QVBoxLayout;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::Node;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;
using QtNodes::Connection;
using ConnectionPolicy = QtNodes::NodeDataModel::ConnectionPolicy;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        struct InNodePort
        {
            InNodePort(QString const& caption_, bool caption_visible_);
            QString caption;
            bool caption_visible = true;
            std::unique_ptr<NodeData> data_type;
            std::weak_ptr<NodeData> in_value;
            QWidget* default_widget = nullptr;
            bool connected = false;
        };

        struct OutNodePort
        {
            OutNodePort(QString const& caption_, bool caption_visible_);
            QString caption;
            bool caption_visible = true;
            std::unique_ptr<NodeData> data_type;
            std::shared_ptr<NodeData> out_value;
            ConnectionPolicy connection_policy = ConnectionPolicy::Many;
            QWidget* default_widget = nullptr;
            bool connected = false;
        };

        enum NodeInterpreterTokens
        {
            NONE,
            BEGIN,
            RETURN,
            RETURN_NO_DATA,
            FOR,
            WHILE,
            BREAK,
            CONTINUE
        };


        class BaseNode : public NodeDataModel
        {
        Q_OBJECT

        public:
            BaseNode();
            virtual ~BaseNode();

        public:

            NodeInterpreterTokens getInterpreterToken() const;
            void setInterpreterToken(NodeInterpreterTokens token);

            unsigned int nPorts(PortType port_type) const override;

            NodeDataType dataType(PortType port_type, PortIndex port_index) const override;

            std::shared_ptr<NodeData> outData(PortIndex port) override;

            void setInData(std::shared_ptr<NodeData> data, PortIndex port_index) override;

            std::unique_ptr<NodeData>& dataModel(PortType port_type, PortIndex port_index);

            QWidget* embeddedWidget() override;

            NodeValidationState validationState() const override;

            QString validationMessage() const override;

            QString portCaption(PortType port_type, PortIndex port_index) const override;

            QString name() const override;

            QString caption() const override;

            void setCaption(QString const& caption);

            bool portCaptionVisible(PortType port_type, PortIndex port_index) const override;

            ConnectionPolicy portOutConnectionPolicy(PortIndex port_index) const override;

            QWidget* portDefaultValueWidget(PortType port_type, PortIndex port_index) override;

            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

            virtual bool isLogicNode();

            void setValidationMessage(QString const& message);
            void setValidationState(NodeValidationState state);

            virtual void compute() = 0;
            virtual NodeValidationState validate();

            bool isComputed() const;
            void setComputed(bool state);

        public Q_SLOTS:

            void inputConnectionCreated(Connection const& connection) override;
            void inputConnectionDeleted(Connection const& connection) override;
            void outputConnectionCreated(Connection const& connection) override;
            void outputConnectionDeleted(Connection const& connection) override;

            void captionDoubleClicked() override;

        protected:

            void setName(QString const& name);

            void addWidgetTop(QWidget* widget);
            void addWidgetBottom(QWidget* widget);
            void addDefaultWidget(QWidget* widget, PortType port_type, PortIndex port_index);



            template<typename T>
            void addPort(PortType port_type,
                         QString const& caption,
                         bool caption_visible,
                         ConnectionPolicy out_policy = ConnectionPolicy::Many);

            template<typename T>
            void addPort(PortType port_type,
                         PortIndex port_index,
                         QString const& caption,
                         bool caption_visible,
                         ConnectionPolicy out_policy = ConnectionPolicy::Many);

            template <typename T>
            void addPortDynamic(PortType port_type,
                         PortIndex port_index,
                         QString const& caption,
                         bool caption_visible,
                         ConnectionPolicy out_policy = ConnectionPolicy::Many);


            template<typename T>
            void addPortDefault(PortType port_type,
                         QString const& caption,
                         bool caption_visible,
                         ConnectionPolicy out_policy = ConnectionPolicy::Many);

            template<typename T>
            void addPortDefault(PortType port_type,
                         PortIndex port_index,
                         QString const& caption,
                         bool caption_visible,
                         ConnectionPolicy out_policy = ConnectionPolicy::Many);

            template <typename T>
            void addPortDefaultDynamic(PortType port_type,
                                      PortIndex port_index,
                                      QString const& caption,
                                      bool caption_visible,
                                      ConnectionPolicy out_policy = ConnectionPolicy::Many);

            void deletePort(PortType port_type, PortIndex port_index);
            void deleteDefaultWidget(PortType port_type, PortIndex port_index);

            void defaultWidgetToJson(PortType port_type, PortIndex port_index, QJsonObject& json_obj, QString const& name) const;
            void defaultWidgetFromJson(PortType port_type, PortIndex port_index, const QJsonObject& json_obj, QString const& name);

            template <typename T>
            std::shared_ptr<T> defaultPortData(PortType port_type, PortIndex port_index);

        protected:

            QString _name;
            QString _caption;

            std::vector<InNodePort> _in_ports;
            std::vector<OutNodePort> _out_ports;

            QWidget _embedded_widget;
            QVBoxLayout* _embedded_widget_layout;
            QVBoxLayout* _embedded_widget_layout_top;
            QVBoxLayout* _embedded_widget_layout_bottom;

            NodeValidationState _validation_state = NodeValidationState::Warning;
            QString _validation_error = QString("Missing or incorrect inputs");

            bool _is_computed = false;

            NodeInterpreterTokens _token = NodeInterpreterTokens::NONE;
        };

    }

}

#endif //NOGGIT_BASENODE_HPP
