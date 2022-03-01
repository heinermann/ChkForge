#include "doodadpalette.h"
#include "ui_doodadpalette.h"

DoodadPalette::DoodadPalette(QWidget *parent) :
  QFrame(parent),
  ui(std::make_unique<Ui::DoodadPalette>())
{
  ui->setupUi(this);
}

DoodadPalette::~DoodadPalette() {}
