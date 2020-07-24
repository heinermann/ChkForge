#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QFrame>

namespace Ui {
  class MapView;
}

class MapView : public QFrame
{
  Q_OBJECT

public:
  explicit MapView(QWidget *parent = nullptr);
  ~MapView();

private:
  Ui::MapView *ui;
};

#endif // MAPVIEW_H
