#include "QUnsignedSpinBox.hpp"

#include <QLineEdit>

#include <limits>

QUnsignedSpinBox::QUnsignedSpinBox(QWidget* parent)
  : m_minimum{std::numeric_limits<quint32>::min()}
  , m_maximum{std::numeric_limits<quint32>::max()}
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    lineEdit()->setText("0");
    lineEdit()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    connect(lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(onEditFinished()));
}

 QUnsignedSpinBox::~QUnsignedSpinBox()
{
}

 quint32 QUnsignedSpinBox::value() const
{
  return m_value;
}

 quint32 QUnsignedSpinBox::minimum() const
{
  return m_minimum;
}

 void QUnsignedSpinBox::setMinimum(quint32 min)
{
  m_minimum = min;
}

 quint32 QUnsignedSpinBox::maximum() const
{
  return m_maximum;
}

 void QUnsignedSpinBox::setMaximum(quint32 max)
{
  m_maximum = max;
}

 void QUnsignedSpinBox::setRange(quint32 min, quint32 max)
{
  setMinimum(min);
  setMaximum(max);
}

 void QUnsignedSpinBox::stepBy(int steps)
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

//bool event(QEvent *event);
 QValidator::State QUnsignedSpinBox::validate(QString& input, int& pos) const
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

 quint32 QUnsignedSpinBox::valueFromText(const QString& text) const
{

  if (text.isEmpty())
    return 0;

  return text.toUInt();
}

 QString QUnsignedSpinBox::textFromValue(quint32 val) const
{
  return QString::number(val);
}

 QAbstractSpinBox::StepEnabled QUnsignedSpinBox::stepEnabled() const
{
  return StepUpEnabled | StepDownEnabled;
}

void QUnsignedSpinBox::setValue(quint32 val)
{
  if (m_value != val) {
    lineEdit()->setText(textFromValue(val));
    m_value = val;
  }
}

 void QUnsignedSpinBox::onEditFinished()
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
