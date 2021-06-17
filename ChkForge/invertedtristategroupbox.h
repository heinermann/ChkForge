#pragma once

#include "tristategroupbox.h"

class InvertedTriStateGroupBox : public TriStateGroupBox {
  Q_OBJECT;
public:
  using TriStateGroupBox::TriStateGroupBox;

  // Used to draw tri-state and invert check state
  void paintEvent(QPaintEvent*) override;
};