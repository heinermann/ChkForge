#include "exportimage.h"
#include "ui_exportimage.h"

ExportImage::ExportImage(QWidget *parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::ExportImage>())
{
  ui->setupUi(this);
}

ExportImage::~ExportImage() {}
