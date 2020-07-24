#ifndef MINIMAP_H
#define MINIMAP_H

#include "DockWidgetWrapper.h"
#include "ui_minimap.h"

class Minimap : public DockWidgetWrapper<Ui::Minimap>
{
  Q_OBJECT

public:
  explicit Minimap(QWidget *parent = nullptr);
  virtual ~Minimap() {}

private:
};

#endif // MINIMAP_H
