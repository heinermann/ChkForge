#include "recenttiles.h"
#include "ui_recenttiles.h"

RecentTiles::RecentTiles(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::RecentTiles)
{
  ui->setupUi(this);
}

RecentTiles::~RecentTiles()
{
  delete ui;
}
