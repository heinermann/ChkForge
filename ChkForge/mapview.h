#ifndef MAPVIEW_H
#define MAPVIEW_H

#include "DockWidgetWrapper.h"
#include "ui_mapview.h"
#include <QImage>
#include <QTimer>
#include <QGraphicsScene>
#include <memory>

#include <QVector>
#include <Qrgb>
#include <QPoint>
#include <QPixmap>

struct main_t;

class MapView : public DockWidgetWrapper<Ui::MapView>
{
  Q_OBJECT

public:
  explicit MapView(QWidget *parent = nullptr);
  virtual ~MapView();

  void init();

  void setScreenPos(const QPoint& pos);
  QPoint getScreenPos();
  QSize getViewSize();

  void move_minimap(int x, int y);
  void draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height);
  QVector<QRgb> get_palette();

  int map_tile_width();
  int map_tile_height();

private:
  QImage buffer;
  QPixmap pix_buffer;
  main_t* bw = nullptr;
  std::unique_ptr<QTimer> timer;

  bool is_paused = false;
  bool is_drag_selecting = false;
  bool is_dragging_screen = false;
  QPoint last_drag_position{};
  QPoint drag_select_from{};
  QPoint drag_select_to{};
  QPoint drag_screen_pos{};

  double view_scale = 1.0;

  void updateScrollbarPositions();

  virtual void changeEvent(QEvent* event) override;

  virtual bool eventFilter(QObject* obj, QEvent* e) override;
  bool surfaceEventFilter(QObject* obj, QEvent* e);
  bool mouseEventFilter(QObject* obj, QEvent* e);
  void paint_surface(QWidget* obj, QPaintEvent* paintEvent);

  void resizeSurface(const QSize& newSize);

  void updateSurface();

private slots:
  void onCloseRequested();
  void updateLogic();
  void hScrollMoved();
  void vScrollMoved();

};

#endif // MAPVIEW_H
