#include "savemap.h"
#include "ui_savemap.h"

SaveMap::SaveMap(QWidget *parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::SaveMap>())
{
  ui->setupUi(this);
}

SaveMap::~SaveMap() {}
