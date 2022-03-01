#pragma once

#include <QDialog>
#include <memory>

namespace Ui {
  class About;
}

class About : public QDialog
{
  Q_OBJECT

public:
  explicit About(QWidget* parent = nullptr);
  ~About();

private:
  std::unique_ptr<Ui::About> ui;
};
