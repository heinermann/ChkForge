#include "savemap.h"
#include "ui_savemap.h"

SaveMap::SaveMap(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SaveMap)
{
  ui->setupUi(this);
}

SaveMap::~SaveMap()
{
  delete ui;
}
