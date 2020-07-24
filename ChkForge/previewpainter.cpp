#include "previewpainter.h"
#include "ui_previewpainter.h"

PreviewPainter::PreviewPainter(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::PreviewPainter)
{
  ui->setupUi(this);
}

PreviewPainter::~PreviewPainter()
{
  delete ui;
}
