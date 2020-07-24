#include "exportsections.h"
#include "ui_exportsections.h"

ExportSections::ExportSections(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ExportSections)
{
  ui->setupUi(this);
}

ExportSections::~ExportSections()
{
  delete ui;
}
