#include "mapview.h"
#include "ui_mapview.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSize>
#include <QWindow>
#include <QPainter>
#include <QImage>
#include <QColor>
#include <QResizeEvent>
#include <QPoint>
#include <QRect>
#include <QMenu>

#include "minimap.h"

#include "../openbw/openbw/ui/ui.h"
#include "../openbw/openbw/bwgame.h"

#include "MapContext.h"
#include "Utils.h"

MapView::MapView(std::shared_ptr<ChkForge::MapContext> mapContext, QWidget *parent)
  : QMdiSubWindow(parent)
  , map(mapContext)
  , ui(std::make_unique<Ui::MapView>())
{
  QFrame* frame = new QFrame();
  ui->setupUi(frame);
  this->setWidget(frame);
  this->setAttribute(Qt::WA_DeleteOnClose);

  updateTitle();

  connect(ui->hScroll, &QScrollBar::valueChanged, this, &MapView::hScrollMoved);
  connect(ui->vScroll, &QScrollBar::valueChanged, this, &MapView::vScrollMoved);

  ui->surface->installEventFilter(this);

  map->add_view(this);

  resizeSurface(ui->surface->size());

  connect(ui->surface, &QOpenGLWidget::customContextMenuRequested, this, &MapView::showContextMenu);
}

MapView::~MapView()
{
  map->remove_view(this);
}

void MapView::closeEvent(QCloseEvent* closeEvent)
{
  if (map->has_one_view() && map->is_unsaved()) {
    auto widget = qobject_cast<ads::CDockWidget*>(sender());

    auto qstring_map_name = QString::fromStdString(map->mapname());
    auto question_str = tr("Do you want to save changes to \"%1\" before closing?").arg(qstring_map_name);

    auto result = QMessageBox::question(this, tr("Unsaved changes..."), question_str,
      QMessageBox::StandardButton::Save | QMessageBox::StandardButton::Discard | QMessageBox::StandardButton::Cancel);

    switch (result) {
    case QMessageBox::Save:
      map->save();
      break;
    case QMessageBox::Discard:
      break;
    case QMessageBox::Cancel:
    default:
      closeEvent->ignore();
      return;
    }
  }
  emit aboutToClose(this);
  closeEvent->accept();
}

QSize MapView::map_tile_size() const
{
  return QSize{ map_tile_width(), map_tile_height() };
}

int MapView::map_tile_width() const
{
  return map->openbw_ui.game_st.map_tile_width;
}

int MapView::map_tile_height() const
{
  return map->openbw_ui.game_st.map_tile_height;
}

void MapView::updateSurface()
{
  this->ui->surface->update();
}

QSize MapView::minimap_size() const
{
  int tile_width = map_tile_width();
  int tile_height = map_tile_height();

  int minimap_width = std::max(tile_width, tile_height);
  int minimap_height = minimap_width;

  if (tile_width < tile_height) {
    minimap_width = minimap_width * minimap_width * tile_width / (minimap_height * tile_height);
  }
  else if (tile_height < tile_width) {
    minimap_height = minimap_height * minimap_height * tile_height / (minimap_width * tile_width);
  }

  return QSize{ minimap_width, minimap_height };
}

void MapView::draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height)
{
  map->openbw_ui.draw_minimap(data, data_pitch, surface_width, surface_height);
}

void MapView::move_minimap(const QPoint& pos)
{
  auto mm_size = minimap_size();

  QPoint clamped_pos = ChkForge::pt{ pos }.clamped({ 0, 0, mm_size.width() - 1, mm_size.height() - 1 });

  int x = clamped_pos.x() * map_tile_size().width() / mm_size.width();
  int y = clamped_pos.y() * map_tile_size().height() / mm_size.height();

  QPoint{ screen_position.width(), screen_position.height() };

  setScreenPos(QPoint{ x, y } * 32 - QPoint{ screen_position.width(), screen_position.height() } / 2);

  updateScrollbarPositions();
  updateSurface();
}

QVector<QRgb> MapView::get_palette() {
  native_window_drawing::color* raw_colorTable = map->openbw_ui.get_palette();
  QVector<QRgb> colors(256);
  for (int i = 0; i < 256; ++i) {
    native_window_drawing::color src_color = raw_colorTable[i];
    colors[i] = qRgb(src_color.r, src_color.g, src_color.b);
  }
  return colors;
}

void MapView::setViewScalePercent(double value)
{
  this->view_scale_percent = std::clamp(value, 20.0, 300.0);
  emit scaleChangedPercent(std::ceil(view_scale_percent));
  resizeSurface(ui->surface->size());
}

double MapView::getViewScale() const {
  return this->view_scale_percent / 100.0;
}

bool MapView::mouseEventFilter(QObject* obj, QEvent* e)
{
  QMouseEvent* mouseEvent = reinterpret_cast<QMouseEvent*>(e);

  if (map->get_layer()->mouseEvent(this, mouseEvent)) return true;

  bool double_clicked = mouseEvent->flags() & Qt::MouseEventCreatedDoubleClick;
  bool shift_pressed = mouseEvent->modifiers() & Qt::ShiftModifier;
  bool ctrl_pressed = mouseEvent->modifiers() & Qt::ControlModifier;
  QPoint map_pos = pointToMap(mouseEvent->pos());

  switch (e->type())
  {
  case QEvent::MouseButtonPress:
    if (mouseEvent->button() == Qt::MiddleButton) {
      this->is_dragging_screen = true;
      setCursor(Qt::ClosedHandCursor);
      this->last_drag_position = mouseEvent->pos();
      return true;
    }
    break;
  case QEvent::MouseButtonDblClick:
    break;
  case QEvent::MouseButtonRelease:
    if (mouseEvent->button() == Qt::MiddleButton) {
      this->is_dragging_screen = false;
      setCursor(Qt::ArrowCursor);
      return true;
    }
    break;
  case QEvent::MouseMove:
    emit mouseMove(mouseEvent->pos());

    if (mouseEvent->buttons() & Qt::MiddleButton) {
      if (is_dragging_screen) {

        QPoint diff = mouseEvent->pos() - this->last_drag_position;
        this->last_drag_position = mouseEvent->pos();

        // TODO: Investigate how to wrap without this line (without bugs)
        if (diff.manhattanLength() > ui->surface->width() / 4) return true;

        this->setScreenPos(pointToMap(-diff));
        updateSurface();

        // Do mouse wrapping
        auto x_view_offset = QPoint{ ui->surface->width() - 8, 0 };
        auto y_view_offset = QPoint{ 0, ui->surface->height() - 8 };

        if (mouseEvent->pos().x() < 0) {
          this->last_drag_position += x_view_offset;
          QCursor::setPos(mouseEvent->globalPosition().toPoint() + x_view_offset);
        }
        else if (mouseEvent->pos().x() >= ui->surface->width()) {
          this->last_drag_position -= x_view_offset;
          QCursor::setPos(mouseEvent->globalPosition().toPoint() - x_view_offset);
        }

        if (mouseEvent->pos().y() < 0) {
          this->last_drag_position += y_view_offset;
          QCursor::setPos(mouseEvent->globalPosition().toPoint() + y_view_offset);
        }
        else if (mouseEvent->pos().y() >= ui->surface->height()) {
          this->last_drag_position -= y_view_offset;
          QCursor::setPos(mouseEvent->globalPosition().toPoint() - y_view_offset);
        }

      }
      return true;
    }
    break;
  }
  return false;
}

bool MapView::surfaceEventFilter(QObject* obj, QEvent* e)
{
  switch (e->type())
  {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove:
    return mouseEventFilter(obj, e);
  case QEvent::Wheel:
  {
    QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(e);
    if (wheelEvent->modifiers() & Qt::KeyboardModifier::ControlModifier) {
      if (wheelEvent->angleDelta().y() / 8 < 0) {
        setViewScalePercent(view_scale_percent * 0.9);
      }
      else if (wheelEvent->angleDelta().y() / 8 > 0) {
        setViewScalePercent(view_scale_percent * 1.1);
      }
    }
    return true;
  }
  case QEvent::Resize:
  {
    QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(e);
    this->resizeSurface(resizeEvent->size());
    return false;
  }
  case QEvent::Paint:
    paint_surface(static_cast<QWidget*>(obj), static_cast<QPaintEvent*>(e));
    return true;
  }

  return false;
}

void MapView::keyPressEvent(QKeyEvent* event)
{
  if (event->modifiers() == Qt::KeyboardModifier::NoModifier) {
    switch (event->key()) {
    case Qt::Key::Key_Left:
      setScreenPos(screen_position.topLeft() - QPoint{ 32, 0 });
      event->accept();
      return;
    case Qt::Key::Key_Right:
      setScreenPos(screen_position.topLeft() + QPoint{ 32, 0 });
      event->accept();
      return;
    case Qt::Key::Key_Up:
      setScreenPos(screen_position.topLeft() - QPoint{ 0, 32 });
      event->accept();
      return;
    case Qt::Key::Key_Down:
      setScreenPos(screen_position.topLeft() + QPoint{ 0, 32 });
      event->accept();
      return;
    }
  }
  QMdiSubWindow::keyPressEvent(event);
}

bool MapView::eventFilter(QObject* obj, QEvent* e)
{
  if (obj == ui->surface) {
    return surfaceEventFilter(obj, e);
  }
  return QMdiSubWindow::eventFilter(obj, e);
}

void MapView::hScrollMoved(int value)
{
  setScreenPos(QPoint{ value, screen_position.y() });
  updateSurface();
}

void MapView::vScrollMoved(int value)
{
  setScreenPos(QPoint{ screen_position.x(), value });
  updateSurface();
}

void MapView::paint_surface(QWidget* obj, QPaintEvent* paintEvent)
{
  QPainter painter;
  painter.begin(obj);
  painter.fillRect(obj->rect(), QColorConstants::Black);
  
  map->openbw_ui.draw_game(this->buffer.bits(), this->buffer.bytesPerLine(), ChkForge::rect{ screen_position });
  
  map->get_layer()->paintGame(this, buffer.bits(), buffer.bytesPerLine(), ChkForge::rect{ screen_position });

  pix_buffer.convertFromImage(this->buffer);
  painter.drawPixmap(obj->rect(), pix_buffer);

  map->get_layer()->paintOverlay(this, obj, painter);

  painter.end();
}

QPoint MapView::getScreenPos()
{
  return screen_position.topLeft();
}

QRect MapView::getScreenRect()
{
  return screen_position;
}

QSize MapView::getViewSize()
{
  return ui->surface->size();
}

void MapView::setScreenPos(const QPoint& pos)
{
  screen_position.moveTopLeft(pos);
  if (screen_position.right() > map->openbw_ui.game_st.map_width) screen_position.moveRight(map->openbw_ui.game_st.map_width);
  if (screen_position.left() < 0) screen_position.moveLeft(0);
  if (screen_position.bottom() > map->openbw_ui.game_st.map_height) screen_position.moveBottom(map->openbw_ui.game_st.map_height);
  if (screen_position.top() < 0) screen_position.moveTop(0);
  updateScrollbarPositions();
}

void MapView::updateScrollbarPositions()
{
  this->ui->hScroll->setValue(screen_position.x());
  this->ui->vScroll->setValue(screen_position.y());
}

void MapView::resizeSurface(QSize newSize)
{
  newSize /= getViewScale();

  this->buffer = QImage(newSize, QImage::Format::Format_Indexed8);
  this->buffer.setColorTable(this->get_palette());

  this->pix_buffer = QPixmap(newSize);

  screen_position.setSize(newSize);
  this->ui->surface->update();

  int hPageStep = newSize.width();
  this->ui->hScroll->setPageStep(hPageStep);
  this->ui->hScroll->setMaximum(map->openbw_ui.game_st.map_width - hPageStep);
  
  int vPageStep = newSize.height();
  this->ui->vScroll->setPageStep(vPageStep);
  this->ui->vScroll->setMaximum(map->openbw_ui.game_st.map_height - vPageStep);
  updateScrollbarPositions();
  updateSurface();
}

void MapView::select_units(bool double_clicked, bool shift, bool ctrl, const QRect &selection)
{
  QRect translated = rectToMap(selection.normalized());
  
  if (!shift) map->openbw_ui.current_selection_clear();

  auto units_to_select = map->find_units(ChkForge::rect{ translated });
  for (bwgame::unit_t* u : units_to_select) {
    map->openbw_ui.current_selection_add(u);
  }

  bwgame::warn("Selected %d units", units_to_select.size());
}

void MapView::select_unit_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position)
{
  QPoint mapPt = pointToMap(position);
  //bwgame::unit_t* u = map->openbw_ui.select_get_unit_at(map->toBw(mapPt));
  bwgame::xy bwPt = ChkForge::pt{ mapPt };
  bwgame::unit_t* u = nullptr;
  for (bwgame::unit_t* found : map->find_units({ bwPt, bwPt })) {
    u = found;
    break;
  }

  if (u == nullptr) {
    if (!shift) map->openbw_ui.current_selection_clear();
    return;
  }

  if (double_clicked || ctrl) {
    if (!shift) map->openbw_ui.current_selection_clear();

    auto is_same_type = [&](bwgame::unit_t* a, bwgame::unit_t* b) {
      if (map->openbw_ui.unit_is_mineral_field(a) && map->openbw_ui.unit_is_mineral_field(b)) return true;
      if (map->openbw_ui.unit_is_normal_tank(a) && map->openbw_ui.unit_is_normal_tank(b)) return true;
      if (map->openbw_ui.unit_is_hero_tank(a) && map->openbw_ui.unit_is_hero_tank(b)) return true;
      return a->unit_type == b->unit_type;
    };

    for (bwgame::unit_t* u2 : map->find_units(ChkForge::rect{ screen_position })) {
      if (u2->owner != u->owner) continue;
      if (!is_same_type(u, u2)) continue;
      map->openbw_ui.current_selection_add(u2);
    }
  }
  else {
    if (shift) {
      map->openbw_ui.current_selection_toggle(u);
    }
    else {
      map->openbw_ui.current_selection_clear();
      map->openbw_ui.current_selection_add(u);
    }
  }

}

std::shared_ptr<ChkForge::MapContext> MapView::getMap()
{
  return map;
}

void MapView::updateTitle()
{
  setWindowTitle(QString::fromStdString(map->filename()) + (map->is_unsaved() ? " *" : ""));
}

QPoint MapView::pointToMap(const QPoint &pt) const
{
  return screen_position.topLeft() + pt / getViewScale();
}

QRect MapView::rectToMap(const QRect &pt) const
{
  return QRect{ pointToMap(pt.topLeft()), pointToMap(pt.bottomRight()) };
}

QPoint MapView::mapToViewPoint(const QPoint& pt) const {
  return (pt - screen_position.topLeft()) * getViewScale();
}

QRect MapView::mapToViewRect(const QRect& rct) const {
  return QRect(mapToViewPoint(rct.topLeft()), mapToViewPoint(rct.bottomRight()));
}

QRect MapView::extendToRect(const QPoint& pt)
{
  return QRect{ pt, pt };
}

void MapView::showContextMenu(const QPoint& pos)
{
  getMap()->get_layer()->showContextMenu(this, pos);
  /*QMenu contextMenu(ui->surface);
  contextMenu.addAction("TODO");
  contextMenu.exec(ui->surface->mapToGlobal(pos));*/
}

void MapView::setItemTreeSelection(ItemTree::Category category, int id)
{
  emit setItemTreeSelectionSignal(category, id);
}
