#include "recenttiles.h"
#include "ui_recenttiles.h"

RecentTiles::RecentTiles(QWidget *parent) :
  QFrame(parent),
  ui(std::make_unique<Ui::RecentTiles>())
{
  ui->setupUi(this);
}

RecentTiles::~RecentTiles() {}
