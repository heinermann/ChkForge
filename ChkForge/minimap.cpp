#include "minimap.h"
#include "ui_minimap.h"

Minimap::Minimap(QWidget *parent) :
  DockWidgetWrapper<Ui::Minimap>("Minimap", parent)
{
}
