#include "doodadpalette.h"
#include "ui_doodadpalette.h"

DoodadPalette::DoodadPalette(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::DoodadPalette)
{
  ui->setupUi(this);
}

DoodadPalette::~DoodadPalette()
{
  delete ui;
}
