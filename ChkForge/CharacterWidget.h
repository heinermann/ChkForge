#pragma once

// Source: https://doc.qt.io/qt-5/qtwidgets-widgets-charactermap-example.html

#include <QWidget>
#include <QSize>
#include <QFont>
#include <QImage>

class CharacterWidget : public QWidget
{
  Q_OBJECT
private:
  struct CharData {
    uint key;
    int category;
    QString str;
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

  QImage virtualRenderer{64, 64, QImage::Format::Format_Mono};

  QFont displayFont;
  int columns = 16;
  int lastIndex = -1;
  int squareSize = 0;
};
