#ifndef NOGGIT_BASENODE_HPP
#define NOGGIT_BASENODE_HPP

#include <external/NodeEditor/include/nodes/NodeDataModel>
#include <external/NodeEditor/include/nodes/NodeData>
#include <external/NodeEditor/include/nodes/Node>
#include <external/NodeEditor/include/nodes/Connection>

#include <vector>
#include <memory>
#include <unordered_map>

#include <QObject>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>

#include <QVBoxLayout>

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
            virtual ~BaseNode() {}

        public:

            NodeInterpreterTokens getInterpreterToken() { return _token; };
            void setInterpreterToken(NodeInterpreterTokens token) { _token = token; };

            unsigned int nPorts(PortType port_type) const override;

            NodeDataType dataType(PortType port_type, PortIndex port_index) const override;

            std::shared_ptr<NodeData> outData(PortIndex port) override;

            void setInData(std::shared_ptr<NodeData> data, PortIndex port_index) override;

            std::unique_ptr<NodeData>& dataModel(PortType port_type, PortIndex port_index);

            QWidget* embeddedWidget() override { return &_embedded_widget; }

            NodeValidationState validationState() const override { return _validation_state; };

            QString validationMessage() const override { return _validation_error; };

            QString portCaption(PortType port_type, PortIndex port_index) const override;

            QString name() const override { return _name; }

            QString caption() const override { return _caption; };

            void setCaption(QString const& caption){_caption = caption;};

            bool portCaptionVisible(PortType port_type, PortIndex port_index) const override;

            ConnectionPolicy portOutConnectionPolicy(PortIndex port_index) const override;

            QWidget* portDefaultValueWidget(PortType port_type, PortIndex port_index) override;

            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

            virtual bool isLogicNode() { return false; };

            void setValidationMessage(QString const& message){_validation_error = message; Q_EMIT visualsNeedUpdate();};
            void setValidationState(NodeValidationState state){_validation_state = state;};

            virtual void compute() = 0;
            virtual NodeValidationState validate() { return _validation_state; };

            bool isComputed() { return _is_computed; };
            void setComputed(bool state) { _is_computed = state; };

        public Q_SLOTS:

            void inputConnectionCreated(Connection const& connection) override;
            void inputConnectionDeleted(Connection const& connection) override;
            void outputConnectionCreated(Connection const& connection) override;
            void outputConnectionDeleted(Connection const& connection) override;

            void captionDoubleClicked() override;

        protected:

            void setName(QString const& name) {_name = name;};

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
