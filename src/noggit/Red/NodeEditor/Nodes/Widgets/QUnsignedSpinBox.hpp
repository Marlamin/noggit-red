// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_QUNSIGNEDSPINBOX_HPP
#define NOGGIT_QUNSIGNEDSPINBOX_HPP

#include <QtWidgets/QWidget>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QLineEdit>

class QUnsignedSpinBoxPrivate;
class Q_WIDGETS_EXPORT QUnsignedSpinBox : public QAbstractSpinBox
{
Q_OBJECT

    Q_PROPERTY(quint32 minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(quint32 maximum READ maximum WRITE setMaximum)

    Q_PROPERTY(quint32 value READ value WRITE setValue NOTIFY valueChanged USER true)


    quint32 m_minimum;
    quint32 m_maximum;
    quint32 m_value = 0;

public:
    explicit QUnsignedSpinBox(QWidget *parent = nullptr)
    {
      setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
      lineEdit()->setText("0");
      lineEdit()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
      connect(lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(onEditFinished()));
    };
    ~QUnsignedSpinBox() {};

    quint32 value() const
    {
      return m_value;
    };

    quint32 minimum() const
    {
      return m_minimum;
    };

    void setMinimum(quint32 min)
    {
      m_minimum = min;
    }

    quint32 maximum() const
    {
      return m_maximum;
    };

    void setMaximum(quint32 max)
    {
      m_maximum = max;
    }

    void setRange(quint32 min, quint32 max)
    {
      setMinimum(min);
      setMaximum(max);
    }

    virtual void stepBy(int steps)
    {
      auto new_value = m_value;
      if (steps < 0 && new_value + steps > new_value) {
        new_value = std::numeric_limits<quint32>::min();
      }
      else if (steps > 0 && new_value + steps < new_value) {
        new_value = std::numeric_limits<quint32>::max();
      }
      else {
        new_value += steps;
      }

      lineEdit()->setText(textFromValue(new_value));
      setValue(new_value);
    }

protected:
    //bool event(QEvent *event);
    virtual QValidator::State validate(QString &input, int &pos) const
    {
      if (input.isEmpty())
        return QValidator::Acceptable;

      bool ok;
      quint32 val = input.toUInt(&ok);
      if (!ok)
        return QValidator::Invalid;

      if (val < m_minimum || val > m_maximum)
        return QValidator::Invalid;

      return QValidator::Acceptable;
    }

    virtual quint32 valueFromText(const QString &text) const
    {

      if (text.isEmpty())
        return 0;

      return text.toUInt();
    }

    virtual QString textFromValue(quint32 val) const
    {
      return QString::number(val);
    }
    //virtual void fixup(QString &str) const;

    virtual QAbstractSpinBox::StepEnabled stepEnabled() const
    {
      return StepUpEnabled | StepDownEnabled;
    }


public Q_SLOTS:
    void setValue(quint32 val)
    {
      if (m_value != val) {
        lineEdit()->setText(textFromValue(val));
        m_value = val;
      }
    }

    void onEditFinished()
    {
      QString input = lineEdit()->text();
      int pos = 0;
      if (QValidator::Acceptable == validate(input, pos))
        setValue(input.isEmpty() ? valueFromText(input) : 0);
      else
        lineEdit()->setText(textFromValue(m_value));

      if (input.isEmpty())
        lineEdit()->setText("0");
    }

Q_SIGNALS:
    void valueChanged(quint32 v);

private:
    Q_DISABLE_COPY(QUnsignedSpinBox)

    Q_DECLARE_PRIVATE(QUnsignedSpinBox)
};

#endif //NOGGIT_QUNSIGNEDSPINBOX_HPP
