#include "previewpainter.h"
#include "ui_previewpainter.h"

PreviewPainter::PreviewPainter(QWidget *parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::PreviewPainter>())
{
  ui->setupUi(this);
}

PreviewPainter::~PreviewPainter() {}
