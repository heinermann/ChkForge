#pragma once
#include "DockWidgetWrapper.h"

#include <QTimer>
#include <QImage>
#include <memory>

class MapView;

namespace Ui {
  class Minimap;
}

class Minimap : public DockWidgetWrapper
{
  Q_OBJECT

public:
  explicit Minimap(QWidget* parent = nullptr);
  virtual ~Minimap() override;

  void setActiveMapView(MapView* view);
  void resetMapBuffer();
  void resetPalette();

  void onCloseMapView(MapView* map);

private:
  std::unique_ptr<Ui::Minimap> ui;

  QImage minimap_buffer;

  std::unique_ptr<QTimer> timer;
  MapView* activeMapView = nullptr;

  virtual bool eventFilter(QObject* obj, QEvent* event) override;
  void paint_surface(QWidget* obj, QPaintEvent* paintEvent);
  void updateLogic();
};
