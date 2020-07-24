#include "outputwindow.h"
#include "ui_outputwindow.h"

OutputWindow::OutputWindow(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::OutputWindow)
{
  ui->setupUi(this);
}

OutputWindow::~OutputWindow()
{
  delete ui;
}
