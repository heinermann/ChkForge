#pragma once

// Source: https://doc.qt.io/qt-5/qtwidgets-widgets-charactermap-example.html

#include <QWidget>
#include <QSize>
#include <QFont>
#include <QImage>
#include <QRawFont>

#include <optional>

class CharacterWidget : public QWidget
{
  Q_OBJECT
private:
  struct FontInfo {
    QFont font;
    QFontMetrics metrics;
    QRawFont rawFont;
  };

  struct CharData {
    uint key;
    QString str;
    FontInfo* font;
    std::optional<QPixmap> cache;
  };
  std::vector<CharData> charlist;

public:
  CharacterWidget(QWidget* parent = nullptr);
  QSize sizeHint() const override;

  void setFont(const QFont& font);

signals:
  void characterSelected(const QString& character);

protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  void calculateSquareSize();
  FontInfo* fontForChar(uint key);
  QChar::Category categoryForName(QString categoryName);

  QList<FontInfo> displayFonts;
  int columns = 16;
  int lastIndex = -1;
  int squareSize = 0;

  struct UniChrData {
    QString desc1;
    QString desc2;
    QChar::Category category;
  };

  std::unordered_map<uint32_t, UniChrData> charDescriptions;
};
