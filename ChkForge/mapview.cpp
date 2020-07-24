#include "mapview.h"
#include "ui_mapview.h"

MapView::MapView(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::MapView)
{
  ui->setupUi(this);
}

MapView::~MapView()
{
  delete ui;
}
