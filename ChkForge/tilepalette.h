#pragma once
#include <QFrame>
#include <memory>

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
  std::unique_ptr<Ui::TilePalette> ui;
};
