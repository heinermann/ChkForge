#pragma once
#include <QFrame>
#include <memory>

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
  std::unique_ptr<Ui::DoodadPalette> ui;
};
