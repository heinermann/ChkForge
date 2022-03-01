#pragma once
#include "DockWidgetWrapper.h"

#include <memory>

namespace Ui {
  class TerrainBrush;
}

class TerrainBrush : public DockWidgetWrapper
{
  Q_OBJECT

public:
  explicit TerrainBrush(QWidget *parent = nullptr);
  virtual ~TerrainBrush() override;

private:
  std::unique_ptr<Ui::TerrainBrush> ui;
};
