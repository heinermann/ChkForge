#pragma once
#include <QFrame>
#include <memory>

namespace Ui {
  class OutputWindow;
}

class OutputWindow : public QFrame
{
  Q_OBJECT

public:
  explicit OutputWindow(QWidget *parent = nullptr);
  ~OutputWindow();

private:
  std::unique_ptr<Ui::OutputWindow> ui;
};
