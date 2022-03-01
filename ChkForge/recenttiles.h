#pragma once
#include <QFrame>
#include <memory>

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
  std::unique_ptr<Ui::RecentTiles> ui;
};
