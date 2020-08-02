#include "about.h"
#include "ui_aboutdlg.h"

About::About(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::About)
{
  ui->setupUi(this);
}

About::~About()
{
  delete ui;
}
