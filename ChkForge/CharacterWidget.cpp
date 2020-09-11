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
#include <QFile>

#include <execution>
#include <algorithm>
#include <numeric>
#include <vector>
#include <set>
#include <iostream>

CharacterWidget::CharacterWidget(QWidget* parent)
  : QWidget(parent)
{
  QFile uniData{":/txt/UnicodeData.txt"};
  uniData.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text);

  QByteArrayList entries = uniData.readAll().split('\n');
  std::set<QString> categories;
  for (const QByteArray& line : entries) {
    if (line.isEmpty()) continue;
    QByteArrayList entry = line.split(';');

    uint32_t key = entry[0].toUInt(nullptr, 16);
    QString desc1 = entry[1];
    QString category = entry[2];
    QString desc2 = entry[10];

    categories.insert(category);

    charDescriptions[key] = std::make_pair(desc1, desc2);
  }
  setFont(QFont());
  //QMessageBox::about(nullptr, "", QString("%1 categories:\n%2").arg(categories.size()).arg(QStringList(categories.begin(), categories.end()).join(", ")));

  calculateSquareSize();
  setMouseTracking(true);
}

std::optional<std::pair<QFont*, QFontMetrics*>> CharacterWidget::fontForChar(uint key) {
  for (auto& fnt : displayFonts) {
    if (fnt.second.inFontUcs4(key)) return std::make_pair(&fnt.first, &fnt.second);
  }
  return std::nullopt;
}


void CharacterWidget::setFont(const QFont& font)
{
  for (QString family : QList{"Eurostile", "BlizzardGlobal", "Malgun Gothic", "UDTypos58B-P", "DejaVu Serif", "MS Gothic"}) {
    QFont font{ family, 11 };
    font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
    font.setStyleHint(QFont::SansSerif, QFont::StyleStrategy(QFont::NoAntialias | QFont::NoSubpixelAntialias | QFont::ForceOutline | QFont::NoFontMerging));
    displayFonts.append({ font, QFontMetrics(font) });
  }

  //QRawFont rawFont = QRawFont::fromFont(displayFont, QFontDatabase::WritingSystem::Greek);

  auto clock = QTime();
  clock.start();

  charlist.clear();
  for (uint key = 1; key < 0x110000; ++key) {
    //if (!rawFont.supportsCharacter(key)) continue;

    auto fnt = fontForChar(key);
    if (!fnt.has_value()) continue;

    int category = -1;
    if (key < 0x10000) {
      category = QChar(key).category();
    }
    else {
      category = 100;
    }
    charlist.emplace_back(CharData{ key, category, QString::fromUcs4(&key, 1), fnt->first, fnt->second });
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

  QRect redrawRect = event->rect();
  int beginRow = redrawRect.top() / squareSize;
  int endRow = redrawRect.bottom() / squareSize;
  int beginColumn = redrawRect.left() / squareSize;
  int endColumn = redrawRect.right() / squareSize;
  
  //QFontMetrics fontMetrics(displayFont, &virtualRenderer);
  QTextOption textOption;
  textOption.setFlags(QTextOption::SuppressColors);
  textOption.setUseDesignMetrics(true);
  textOption.setWrapMode(QTextOption::NoWrap);

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

    QPoint textPos{ rct.center().x() - chr.metrics->horizontalAdvance(chr.str) / 2, rct.y() + 3 + chr.metrics->ascent() };
    painter.setPen(QPen(Qt::black));
    painter.setFont(*chr.srcFont);
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

  auto& description = charDescriptions[chr.key];

  QString text = QString::fromLatin1("<p><span style=\"font-size: 24pt; font-family: %1\">").arg(chr.srcFont->family())
    + chr.str
    + QString("</span><p>Value: 0x") + QString::number(chr.key, 16)
    + QString("<br>Family: ") + chr.srcFont->family();

  if (!description.first.isEmpty())
    text += QString("<br>") + description.first;

  if (!description.second.isEmpty())
    text += QString("<br>") + description.second;

  QToolTip::showText(event->globalPos(), text, this);
}

void CharacterWidget::calculateSquareSize()
{
  QFontMetrics& metrics = displayFonts.first().second;
  squareSize = qMax(16, qMax(4 + metrics.height(), 4 + metrics.maxWidth()));
}
