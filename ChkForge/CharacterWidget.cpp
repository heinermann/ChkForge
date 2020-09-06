#include "CharacterWidget.h"

#include <QPainter>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QBrush>
#include <QToolTip>
#include <QRect>
#include <QTime>
#include <QRawFont>
#include <QMessageBox>
#include <QGuiApplication>
#include <QClipboard>

#include <execution>
#include <algorithm>
#include <numeric>
#include <vector>
#include <iostream>

CharacterWidget::CharacterWidget(QWidget* parent)
  : QWidget(parent)
{
  calculateSquareSize();
  setMouseTracking(true);
}

void CharacterWidget::setFont(const QFont& font)
{
  displayFont.setFamily(font.family());
  QFontMetrics fontMetrics(displayFont, &virtualRenderer);
  QRawFont rawFont = QRawFont::fromFont(displayFont, QFontDatabase::WritingSystem::Greek);
  
  auto clock = QTime();
  clock.start();

  charlist.clear();
  for (uint key = 0; key < 0x110000; ++key) {
    //if (!rawFont.supportsCharacter(key)) continue;
    if (!fontMetrics.inFontUcs4(key)) continue;

    int category = -1;
    if (key < 0x10000) {
      category = QChar(key).category();
    }
    else {
      category = 100;
    }
    charlist.emplace_back(CharData{ key, category, QString::fromUcs4(&key, 1) });
  }

  double s = clock.elapsed() / 1000.0;
  QMessageBox::about(nullptr, "", QString("Took %1s").arg(s));

  calculateSquareSize();
  adjustSize();
  update();
}

QSize CharacterWidget::sizeHint() const
{
  return QSize(columns * squareSize, ((charlist.size() + columns - 1) / columns) * squareSize);
}

void CharacterWidget::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.fillRect(event->rect(), QBrush(Qt::white));
  painter.setFont(displayFont);

  QRect redrawRect = event->rect();
  int beginRow = redrawRect.top() / squareSize;
  int endRow = redrawRect.bottom() / squareSize;
  int beginColumn = redrawRect.left() / squareSize;
  int endColumn = redrawRect.right() / squareSize;
  
  /*
  painter.setPen(QPen(Qt::gray));
  for (int row = beginRow; row <= endRow; ++row) {
    for (int column = beginColumn; column <= endColumn; ++column) {
      painter.drawRect(column * squareSize, row * squareSize, squareSize, squareSize);
    }
  }*/

  QFontMetrics fontMetrics(displayFont, &virtualRenderer);
  painter.setPen(QPen(Qt::black));
  for (int i = beginRow * columns + beginColumn; i <= endRow * columns + endColumn; ++i) {
    if (i >= charlist.size()) break;

    const auto& chr = charlist[i];

    int row = i / columns;
    int col = i % columns;
    QRect rct{ col * squareSize, row * squareSize, squareSize, squareSize };

    painter.setClipRect(rct);
    painter.setPen(QPen(Qt::gray));
    painter.drawRect(rct);

    rct.adjust(1, 1, 0, 0);
    painter.setClipRect(rct);

    if (i == lastIndex) {
      painter.fillRect(rct, QBrush(Qt::red));
    }

    QPoint textPos{ rct.center().x() - fontMetrics.horizontalAdvance(chr.str) / 2, rct.y() + 3 + fontMetrics.ascent() };
    painter.setPen(QPen(Qt::black));
    painter.drawText(textPos, chr.str);
  }
}

void CharacterWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    lastIndex = (event->y() / squareSize) * columns + event->x() / squareSize;
    if (lastIndex >= 0 && lastIndex < charlist.size()) {
      emit characterSelected(charlist[lastIndex].str);
      QGuiApplication::clipboard()->setText(charlist[lastIndex].str);
    }
    update();
  }
  else {
    QWidget::mousePressEvent(event);
  }
}

void CharacterWidget::mouseMoveEvent(QMouseEvent* event)
{
  QPoint widgetPosition = mapFromGlobal(event->globalPos());
  uint index = (widgetPosition.y() / squareSize) * columns + widgetPosition.x() / squareSize;
  if (index >= charlist.size()) {
    QToolTip::hideText();
    return;
  }

  const auto& chr = charlist[index];

  QString text = QString::fromLatin1("<p><span style=\"font-size: 24pt; font-family: %1\">").arg(displayFont.family())
    + chr.str
    + QString::fromLatin1("</span><p>Value: 0x")
    + QString::number(chr.key, 16);

  QToolTip::showText(event->globalPos(), text, this);
}

void CharacterWidget::calculateSquareSize()
{
  QFontMetrics metrics{ displayFont, &virtualRenderer };
  squareSize = qMax(16, qMax(4 + metrics.height(), 4 + metrics.maxWidth()));
}
