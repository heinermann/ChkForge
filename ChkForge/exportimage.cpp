#include "exportimage.h"
#include "ui_exportimage.h"

ExportImage::ExportImage(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ExportImage)
{
  ui->setupUi(this);
}

ExportImage::~ExportImage()
{
  delete ui;
}
