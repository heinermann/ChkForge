#ifndef MINIMAP_H
#define MINIMAP_H

#include "DockWidgetWrapper.h"
#include "ui_minimap.h"

#include <QTimer>
#include <QImage>

class MapView;

class Minimap : public DockWidgetWrapper<Ui::Minimap>
{
  Q_OBJECT

public:
  explicit Minimap(QWidget* parent = nullptr);
  virtual ~Minimap();

  void setActiveMapView(MapView* view);
  void resetMapBuffer();
  void resetPalette();

  void onCloseMapView(MapView* map);

private:
  QImage minimap_buffer;

  std::unique_ptr<QTimer> timer;
  MapView* activeMapView = nullptr;

  virtual bool eventFilter(QObject* obj, QEvent* event) override;
  void paint_surface(QWidget* obj, QPaintEvent* paintEvent);
  void updateLogic();
};

#endif // MINIMAP_H
