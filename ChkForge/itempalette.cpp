#include "itempalette.h"
#include "ui_itempalette.h"

ItemPalette::ItemPalette(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::ItemPalette)
{
  ui->setupUi(this);
}

ItemPalette::~ItemPalette()
{
  delete ui;
}
