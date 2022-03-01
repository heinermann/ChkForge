#include "tilepalette.h"
#include "ui_tilepalette.h"

TilePalette::TilePalette(QWidget *parent) :
  QFrame(parent),
  ui(std::make_unique<Ui::TilePalette>())
{
  ui->setupUi(this);
}

TilePalette::~TilePalette() {}
