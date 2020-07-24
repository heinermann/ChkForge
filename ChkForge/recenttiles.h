#ifndef RECENTTILES_H
#define RECENTTILES_H

#include <QFrame>

namespace Ui {
  class RecentTiles;
}

class RecentTiles : public QFrame
{
  Q_OBJECT

public:
  explicit RecentTiles(QWidget *parent = nullptr);
  ~RecentTiles();

private:
  Ui::RecentTiles *ui;
};

#endif // RECENTTILES_H
