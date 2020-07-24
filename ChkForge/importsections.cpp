#include "importsections.h"
#include "ui_importsections.h"

ImportSections::ImportSections(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ImportSections)
{
  ui->setupUi(this);
}

ImportSections::~ImportSections()
{
  delete ui;
}
