#pragma once

// Source: https://doc.qt.io/qt-5/qtwidgets-widgets-charactermap-example.html

#include <QWidget>
#include <QSize>
#include <QFont>
#include <QImage>

#include <optional>

class CharacterWidget : public QWidget
{
  Q_OBJECT
private:
  struct CharData {
    uint key;
    int category;
    QString str;
    QFont* srcFont;
    QFontMetrics* metrics;
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
  std::optional<std::pair<QFont*, QFontMetrics*>> fontForChar(uint key);

  QImage virtualRenderer{64, 64, QImage::Format::Format_Mono};

  QList<std::pair<QFont, QFontMetrics>> displayFonts;
  int columns = 16;
  int lastIndex = -1;
  int squareSize = 0;
  std::unordered_map<uint32_t, std::pair<QString, QString>> charDescriptions;
};
