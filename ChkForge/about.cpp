#include "about.h"
#include "ui_aboutdlg.h"

About::About(QWidget* parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::About>())
{
  ui->setupUi(this);
}

About::~About() {}
