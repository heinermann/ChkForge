#include "tristategroupbox.h"

#include <QStylePainter>
#include <QStyleOptionGroupBox>

TriStateGroupBox::TriStateGroupBox(QWidget* parent)
  : QGroupBox(parent)
{
  init();
}

TriStateGroupBox::TriStateGroupBox(const QString& title, QWidget* parent)
  : QGroupBox(title, parent)
{
  init();
}

void TriStateGroupBox::init() {
  connect(this, &QGroupBox::clicked, this, &TriStateGroupBox::wasClicked);
}

void TriStateGroupBox::paintEvent(QPaintEvent*)
{
  // Get style options from GroupBox parent
  QStylePainter paint(this);
  QStyleOptionGroupBox option;
  initStyleOption(&option);

  // Invert the checkbox state
  if (this->partiallyChecked) {
    option.state &= ~(QStyle::State_On | QStyle::State_Off);
    option.state |= QStyle::StateFlag::State_NoChange;
  }

  paint.drawComplexControl(QStyle::CC_GroupBox, option);
}

void TriStateGroupBox::setPartiallyChecked(bool partialCheck) {
  this->partiallyChecked = partialCheck;
  if (partialCheck) {
    this->setChecked(true);
  }
  update();
}

bool TriStateGroupBox::isPartiallyChecked() {
  return partiallyChecked;
}

void TriStateGroupBox::wasClicked(bool checked) {
  if (partiallyChecked) {
    partiallyChecked = false;
    this->setChecked(true);
  }
}
