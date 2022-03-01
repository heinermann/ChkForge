#pragma once
#include <QFrame>
#include <memory>

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
  std::unique_ptr<Ui::ItemPalette> ui;
};
