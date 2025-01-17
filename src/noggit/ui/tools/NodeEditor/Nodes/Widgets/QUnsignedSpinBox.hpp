// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_QUNSIGNEDSPINBOX_HPP
#define NOGGIT_QUNSIGNEDSPINBOX_HPP

#include <QtWidgets/QAbstractSpinBox>

class QUnsignedSpinBoxPrivate;
class QUnsignedSpinBox : public QAbstractSpinBox
{
Q_OBJECT

    Q_PROPERTY(quint32 minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(quint32 maximum READ maximum WRITE setMaximum)

    Q_PROPERTY(quint32 value READ value WRITE setValue NOTIFY valueChanged USER true)


    quint32 m_minimum;
    quint32 m_maximum;
    quint32 m_value = 0;

public:
    explicit QUnsignedSpinBox(QWidget *parent = nullptr);
    ~QUnsignedSpinBox();

    quint32 value() const;

    quint32 minimum() const;

    void setMinimum(quint32 min);

    quint32 maximum() const;

    void setMaximum(quint32 max);

    void setRange(quint32 min, quint32 max);

    virtual void stepBy(int steps);

protected:
    //bool event(QEvent *event);
    virtual QValidator::State validate(QString &input, int &pos) const;

    virtual quint32 valueFromText(const QString &text) const;

    virtual QString textFromValue(quint32 val) const;
    //virtual void fixup(QString &str) const;

    virtual QAbstractSpinBox::StepEnabled stepEnabled() const;


public Q_SLOTS:
  void setValue(quint32 val);

    void onEditFinished();

Q_SIGNALS:
    void valueChanged(quint32 v);

private:
    Q_DISABLE_COPY(QUnsignedSpinBox)

    Q_DECLARE_PRIVATE(QUnsignedSpinBox)
};

#endif //NOGGIT_QUNSIGNEDSPINBOX_HPP
