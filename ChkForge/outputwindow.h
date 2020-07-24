#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <QFrame>

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
  Ui::OutputWindow *ui;
};

#endif // OUTPUTWINDOW_H
