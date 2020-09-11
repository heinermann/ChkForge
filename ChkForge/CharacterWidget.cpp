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
  for (const QByteArray& line : entries) {
    if (line.isEmpty()) continue;
    QByteArrayList entry = line.split(';');

    uint32_t key = entry[0].toUInt(nullptr, 16);
    QString desc1 = entry[1];
    QString category = entry[2];
    QString desc2 = entry[10];

    charDescriptions[key] = UniChrData{ desc1, desc2, categoryForName(category) };
  }
  setFont(QFont());

  calculateSquareSize();
  setMouseTracking(true);
}

CharacterWidget::FontInfo* CharacterWidget::fontForChar(uint key) {
  for (auto& fnt : displayFonts) {
    if (fnt.rawFont.supportsCharacter(key)) return &fnt;
    //if (fnt.metrics.inFontUcs4(key)) return &fnt;
  }
  return nullptr;
}

QChar::Category CharacterWidget::categoryForName(QString categoryName) {
  static std::unordered_map<QString, QChar::Category> category_map = {
    { "Mn", QChar::Mark_NonSpacing },
    { "Mc", QChar::Mark_SpacingCombining },
    { "Me", QChar::Mark_Enclosing },
    { "Nd", QChar::Number_DecimalDigit },
    { "Nl", QChar::Number_Letter },
    { "No", QChar::Number_Other },
    { "Zs", QChar::Separator_Space },
    { "Zl", QChar::Separator_Line },
    { "Zp", QChar::Separator_Paragraph },
    { "Cc", QChar::Other_Control },
    { "Cf", QChar::Other_Format },
    { "Cs", QChar::Other_Surrogate },
    { "Co", QChar::Other_PrivateUse },
    { "Cn", QChar::Other_NotAssigned },
    { "Lu", QChar::Letter_Uppercase },
    { "Ll", QChar::Letter_Lowercase },
    { "Lt", QChar::Letter_Titlecase },
    { "Lm", QChar::Letter_Modifier },
    { "Lo", QChar::Letter_Other },
    { "Pc", QChar::Punctuation_Connector },
    { "Pd", QChar::Punctuation_Dash },
    { "Ps", QChar::Punctuation_Open },
    { "Pe", QChar::Punctuation_Close },
    { "Pi", QChar::Punctuation_InitialQuote },
    { "Pf", QChar::Punctuation_FinalQuote },
    { "Po", QChar::Punctuation_Other },
    { "Sm", QChar::Symbol_Math },
    { "Sc", QChar::Symbol_Currency },
    { "Sk", QChar::Symbol_Modifier },
    { "So", QChar::Symbol_Other }
  };

  if (category_map.count(categoryName) != 0) {
    return category_map[categoryName];
  }
  return QChar::Other_NotAssigned;
}

void CharacterWidget::setFont(const QFont& font)
{
  // Init fontlist
  for (QString family : QList{"Eurostile", "BlizzardGlobal", "Malgun Gothic", "UDTypos58B-P", "DejaVu Serif", "MS Gothic"}) {
    QFont font{ family, 11 };
    font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
    font.setStyleHint(QFont::SansSerif, QFont::StyleStrategy(QFont::NoAntialias | QFont::NoSubpixelAntialias | QFont::ForceOutline | QFont::NoFontMerging));
    displayFonts.append({ font, QFontMetrics(font), QRawFont::fromFont(font) });
  }

  // Init charlist
  charlist.clear();
  for (uint key = 1; key < 0x110000; ++key) {
    auto fnt = fontForChar(key);
    if (fnt == nullptr) continue;

    charlist.emplace_back(CharData{ key, QString::fromUcs4(&key, 1), fnt });
  }

  // end
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

    QPoint textPos{ rct.center().x() - chr.font->metrics.horizontalAdvance(chr.str) / 2, rct.y() + 3 + chr.font->metrics.ascent() };
    painter.setPen(QPen(Qt::black));
    painter.setFont(chr.font->font);
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

  QString text = QString("<p><span style=\"font-size: 24pt; font-family: %1;\">").arg(chr.font->font.family())
    + chr.str
    + QString("</span><br>Value: U+") + QString::number(chr.key, 16)
    + QString("<br>Family: ") + chr.font->font.family();

  if (!description.desc1.isEmpty())
    text += QString("<br>") + description.desc1;

  if (!description.desc2.isEmpty())
    text += QString("<br>") + description.desc2;

  QToolTip::showText(event->globalPos(), text, this);
}

void CharacterWidget::calculateSquareSize()
{
  QFontMetrics& metrics = displayFonts.first().metrics;
  squareSize = qMax(16, qMax(4 + metrics.height(), 4 + metrics.maxWidth()));
}
