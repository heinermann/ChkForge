#ifndef TILEPALETTE_H
#define TILEPALETTE_H

#include <QFrame>

namespace Ui {
  class TilePalette;
}

class TilePalette : public QFrame
{
  Q_OBJECT

public:
  explicit TilePalette(QWidget *parent = nullptr);
  ~TilePalette();

private:
  Ui::TilePalette *ui;
};

#endif // TILEPALETTE_H
