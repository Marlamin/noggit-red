#ifndef NOGGIT_BASENODE_HPP
#define NOGGIT_BASENODE_HPP

#include <external/NodeEditor/include/nodes/NodeDataModel>
#include <external/NodeEditor/include/nodes/NodeData>
#include <external/NodeEditor/include/nodes/Connection>
#include <boost/variant.hpp>

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
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;
using QtNodes::Connection;
using ConnectionPolicy = QtNodes::NodeDataModel::ConnectionPolicy;


namespace noggit
{
    namespace Red::PresetEditor::Nodes
    {
        struct InNodePort
        {
            InNodePort(QString const& caption_, bool caption_visible_);
            QString caption;
            bool caption_visible = true;
            std::unique_ptr<NodeData> data_type;
            std::weak_ptr<NodeData> in_value;
            QWidget* default_widget = nullptr;
        };

        struct OutNodePort
        {
            OutNodePort(QString const& caption_, bool caption_visible_);
            QString caption;
            bool caption_visible = true;
            std::unique_ptr<NodeData> data_type;
            std::shared_ptr<NodeData> out_value;
            ConnectionPolicy connection_policy = ConnectionPolicy::Many;
        };


        class BaseNode : public NodeDataModel
        {
        Q_OBJECT

        public:
            BaseNode();
            virtual ~BaseNode() {}

        public:

            unsigned int nPorts(PortType port_type) const override;

            NodeDataType dataType(PortType port_type, PortIndex port_index) const override;

            std::shared_ptr<NodeData> outData(PortIndex port) override;

            void setInData(std::shared_ptr<NodeData> data, PortIndex port_index) override;

            QWidget* embeddedWidget() override { return &_embedded_widget; }

            NodeValidationState validationState() const override { return _validation_state; };

            QString validationMessage() const override { return _validation_error; };

            QString portCaption(PortType port_type, PortIndex port_index) const override;

            QString name() const override { return _name; }

            QString caption() const override { return _caption; };

            bool portCaptionVisible(PortType port_type, PortIndex port_index) const override;

            ConnectionPolicy portOutConnectionPolicy(PortIndex port_index) const override;

            QWidget* portDefaultValueWidget(PortIndex port_index) override;

            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

            virtual bool isLogicNode() { return _is_logic_node; };
            void setIsLogicNode(bool state) { _is_logic_node = state; };

            virtual unsigned int nLogicBranches() { return _n_logic_branches; };
            void setNLogicBranches(int n_branches) { _n_logic_branches = n_branches; };

            virtual bool isComputed() { return _computed; };
            void setComputed(bool state) { _computed = state; };

            int logicBranchToExecute() { return _current_logic_branch; };
            void setLogicBranchToExecute(int branch) { _current_logic_branch = branch; };

            void setValidationMessage(QString const& message){_validation_error = message;};
            void setValidationState(NodeValidationState state){_validation_state = state;};

            virtual void compute() = 0;

        public Q_SLOTS:

            void inputConnectionCreated(Connection const& connection) override;
            void inputConnectionDeleted(Connection const& connection) override;

        protected:

            void setName(QString const& name) {_name = name;};
            void setCaption(QString const& caption){_caption = caption;};
            void addWidget(QWidget* widget, PortIndex in_port = -1);
            void addWidget(QWidget* widget, QString const& label_text, PortIndex in_port = -1);

            template<typename T>
            void addPort(PortType port_type,
                         QString const& caption,
                         bool caption_visible,
                         ConnectionPolicy out_policy = ConnectionPolicy::Many);

        protected:

            QString _name;
            QString _caption;

            std::vector<InNodePort> _in_ports;
            std::vector<OutNodePort> _out_ports;

            QWidget _embedded_widget;
            QVBoxLayout* _embedded_widget_layout;

            NodeValidationState _validation_state = NodeValidationState::Warning;
            QString _validation_error = QString("Missing or incorrect inputs");

            bool _is_logic_node = false;
            unsigned int _n_logic_branches = 1;
            int _current_logic_branch = 1;
            bool _computed = false;
        };

    }

}

#endif //NOGGIT_BASENODE_HPP
