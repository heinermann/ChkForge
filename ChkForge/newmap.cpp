#include "newmap.h"
#include "ui_newmap.h"

NewMap::NewMap(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NewMap)
{
  ui->setupUi(this);
}

NewMap::~NewMap()
{
  delete ui;
}
