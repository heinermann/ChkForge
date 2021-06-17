#include "invertedtristategroupbox.h"

#include <QStylePainter>
#include <QStyleOptionGroupBox>

void InvertedTriStateGroupBox::paintEvent(QPaintEvent*)
{
  // Get style options from GroupBox parent
  QStylePainter paint(this);
  QStyleOptionGroupBox option;
  initStyleOption(&option);

  // Invert the checkbox state
  if (this->partiallyChecked) {
    option.state &= ~(QStyle::State_On | QStyle::State_Off);
    option.state |= QStyle::StateFlag::State_NoChange;
  } else if (option.state & (QStyle::State_On | QStyle::State_Off)) {
    option.state ^= QStyle::State_On | QStyle::State_Off;
  }

  paint.drawComplexControl(QStyle::CC_GroupBox, option);
}
