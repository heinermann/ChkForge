#pragma once
#include <QImage>
#include <memory>
#include <optional>

#include <QVector>
#include <Qrgb>
#include <QPoint>
#include <QPixmap>
#include <QMdiSubwindow>

#include "itemtree.h"

namespace ChkForge {
  class MapContext;
}

namespace Ui {
  class MapView;
}

class MapView : public QMdiSubWindow
{
  Q_OBJECT

public:
  explicit MapView(std::shared_ptr<ChkForge::MapContext> mapContext, QWidget *parent = nullptr);
  virtual ~MapView();

  void setScreenPos(const QPoint& pos);
  QPoint getScreenPos();
  QRect getScreenRect();
  QSize getViewSize();

  void move_minimap(const QPoint& pos);
  void draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height);
  QVector<QRgb> get_palette();

  QSize map_tile_size() const;
  int map_tile_width() const;
  int map_tile_height() const;

  QSize minimap_size() const;

  void select_units(bool double_clicked, bool shift, bool ctrl, const QRect &selection);
  void select_unit_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position);

  void updateTitle();
  void updateSurface();

  void setViewScalePercent(double value);
  double getViewScale();

  QPoint pointToMap(const QPoint &pt);
  QRect rectToMap(const QRect &pt);

  QRect extendToRect(const QPoint &pt);

  std::shared_ptr<ChkForge::MapContext> getMap();

  void setItemTreeSelection(ItemTree::Category category, int id);
private:
  std::unique_ptr<Ui::MapView> ui;

  QImage buffer;
  QPixmap pix_buffer;
  std::shared_ptr<ChkForge::MapContext> map;

  bool is_paused = false;
  bool is_dragging_screen = false;
  QPoint last_drag_position{};
  std::optional<QRect> drag_select = std::nullopt;
  QPoint drag_screen_pos{};

  QRect screen_position{0, 0, 640, 480};

  double view_scale_percent = 100.0;

private:
  void updateScrollbarPositions();

  virtual bool eventFilter(QObject* obj, QEvent* e) override;
  bool surfaceEventFilter(QObject* obj, QEvent* e);
  bool mouseEventFilter(QObject* obj, QEvent* e);
  void paint_surface(QWidget* obj, QPaintEvent* paintEvent);

  void resizeSurface(QSize newSize);

  virtual void keyPressEvent(QKeyEvent* event) override;

  virtual void closeEvent(QCloseEvent* closeEvent) override;
  
  void hScrollMoved(int value);
  void vScrollMoved(int value);

  void showContextMenu(const QPoint& pos);

signals:
  void aboutToClose(MapView* map);
  void mouseMove(const QPoint& pt);
  void scaleChangedPercent(int value);
  void setItemTreeSelectionSignal(ItemTree::Category category, int id);
};
