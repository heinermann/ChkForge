#include "tristategroupbox.h"

#include <QStylePainter>
#include <QStyleOptionGroupBox>

void TriStateGroupBox::paintEvent(QPaintEvent*)
{
  // Get style options from GroupBox parent
  QStylePainter paint(this);
  QStyleOptionGroupBox option;
  initStyleOption(&option);

  // Invert the checkbox state
  if (option.state & (QStyle::State_On | QStyle::State_Off)) {
    option.state ^= QStyle::State_On | QStyle::State_Off;
  }

  // Draw control
  paint.drawComplexControl(QStyle::CC_GroupBox, option);
}
