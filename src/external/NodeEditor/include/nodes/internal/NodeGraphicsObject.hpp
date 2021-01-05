#pragma once

#include <QtCore/QUuid>
#include <QtWidgets/QGraphicsObject>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>

#include "Connection.hpp"

#include "NodeGeometry.hpp"
#include "NodeState.hpp"

namespace QtNodes
{

class FlowScene;
class FlowItemEntry;

class ProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT

public:
    ProxyWidget(QGraphicsItem* parent = nullptr) : QGraphicsProxyWidget(parent) {};

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override
    {
      QGraphicsProxyWidget::mousePressEvent(event);
      event->accept();
    };
};

/// Class reacts on GUI events, mouse clicks and
/// forwards painting operation.
class NodeGraphicsObject : public QGraphicsObject
{
  Q_OBJECT

public:
  NodeGraphicsObject(FlowScene &scene,
                     Node& node);

  virtual
  ~NodeGraphicsObject();

  Node&
  node();

  Node const&
  node() const;

  QRectF
  boundingRect() const override;

  void
  setGeometryChanged();

  /// Visits all attached connections and corrects
  /// their corresponding end points.
  void
  moveConnections() const;

  enum { Type = UserType + 1 };

  int
  type() const override { return Type; }

  void
  lock(bool locked);

protected:
  void
  paint(QPainter*                       painter,
        QStyleOptionGraphicsItem const* option,
        QWidget*                        widget = 0) override;

  QVariant
  itemChange(GraphicsItemChange change, const QVariant &value) override;

  void
  mousePressEvent(QGraphicsSceneMouseEvent* event) override;

  void
  mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

  void
  mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

  void
  hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

  void
  hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

  void
  hoverMoveEvent(QGraphicsSceneHoverEvent *) override;

  void
  mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

  void
  contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
  void
  embedQWidget();

private:

  FlowScene & _scene;

  Node& _node;

  bool _locked;

  // either nullptr or owned by parent QGraphicsItem
  QGraphicsProxyWidget * _proxyWidget;
};
}
