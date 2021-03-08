#pragma once

#include <QGroupBox>
#include <QWidget>
#include <QString>

class TriStateGroupBox : public QGroupBox {
  Q_OBJECT

public:
  using QGroupBox::QGroupBox;

  // Used to invert the checkbox and draw tri-state
  void paintEvent(QPaintEvent*) override;
};
