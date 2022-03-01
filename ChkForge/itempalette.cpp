#include "itempalette.h"
#include "ui_itempalette.h"

ItemPalette::ItemPalette(QWidget *parent) :
  QFrame(parent),
  ui(std::make_unique<Ui::ItemPalette>())
{
  ui->setupUi(this);
}

ItemPalette::~ItemPalette() {}
