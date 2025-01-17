#pragma once

#include <QWidget>

class QEvent;
class QMouseEvent;

class DynamicMouseWidget : public QWidget
{
    Q_OBJECT

public:
    DynamicMouseWidget(QWidget* parent = nullptr);
    virtual ~DynamicMouseWidget();

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};

