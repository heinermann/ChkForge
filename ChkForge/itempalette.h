#ifndef ITEMPALETTE_H
#define ITEMPALETTE_H

#include <QFrame>

namespace Ui {
  class ItemPalette;
}

class ItemPalette : public QFrame
{
  Q_OBJECT

public:
  explicit ItemPalette(QWidget *parent = nullptr);
  ~ItemPalette();

private:
  Ui::ItemPalette *ui;
};

#endif // ITEMPALETTE_H
