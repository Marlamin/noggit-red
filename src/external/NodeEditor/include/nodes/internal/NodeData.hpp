#pragma once

#include <QtCore/QString>
#include <QWidget>
#include <QJsonObject>

#include "Export.hpp"

namespace QtNodes
{

struct NodeDataType
{
  QString id;
  QString name;
};

/// Class represents data transferred between nodes.
/// @param type is used for comparing the types
/// The actual data is stored in subtypes
class NODE_EDITOR_PUBLIC NodeData
{
public:

  virtual ~NodeData() = default;

  virtual bool sameType(NodeData const &nodeData) const
  {
    return (this->type().id == nodeData.type().id);
  }

  /// Type for inner use
  virtual NodeDataType type() const = 0;
  virtual std::unique_ptr<NodeData> instantiate() = 0;
  virtual QWidget* default_widget(QWidget* parent) = 0;
  virtual std::shared_ptr<NodeData> default_widget_data(QWidget* widget) = 0;
  virtual void to_json(QWidget* widget, QJsonObject& json_obj, const std::string& name) = 0;
  virtual void from_json(QWidget* widget, const QJsonObject& json_obj, const std::string& name) = 0;
};
}
