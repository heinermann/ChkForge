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
  explicit Minimap(QWidget *parent = nullptr);
  virtual ~Minimap();

  void setActiveMapView(MapView* view);
  void removeMyMapView(MapView* view);
  void resetMapBuffer();
  void resetPalette();

  void SDLInit();

  static Minimap* g_minimap;
private:
  QImage minimap_buffer{ 256, 256, QImage::Format::Format_Indexed8 };

  std::unique_ptr<QTimer> timer;
  MapView* activeMapView = nullptr;

  virtual bool eventFilter(QObject* obj, QEvent* event) override;
  void paint_minimap(QWidget* obj, QPaintEvent* paintEvent);

private slots:
  void updateLogic();
};

#endif // MINIMAP_H
