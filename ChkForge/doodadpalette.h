#ifndef DOODADPALETTE_H
#define DOODADPALETTE_H

#include <QFrame>

namespace Ui {
  class DoodadPalette;
}

class DoodadPalette : public QFrame
{
  Q_OBJECT

public:
  explicit DoodadPalette(QWidget *parent = nullptr);
  ~DoodadPalette();

private:
  Ui::DoodadPalette *ui;
};

#endif // DOODADPALETTE_H
