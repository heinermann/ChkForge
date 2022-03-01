#include "outputwindow.h"
#include "ui_outputwindow.h"

OutputWindow::OutputWindow(QWidget *parent) :
  QFrame(parent),
  ui(std::make_unique<Ui::OutputWindow>())
{
  ui->setupUi(this);
}

OutputWindow::~OutputWindow() {}
