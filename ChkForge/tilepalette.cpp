#include "tilepalette.h"
#include "ui_tilepalette.h"

TilePalette::TilePalette(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::TilePalette)
{
  ui->setupUi(this);
}

TilePalette::~TilePalette()
{
  delete ui;
}
