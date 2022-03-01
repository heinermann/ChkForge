#include "importsections.h"
#include "ui_importsections.h"

ImportSections::ImportSections(QWidget *parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::ImportSections>())
{
  ui->setupUi(this);
}

ImportSections::~ImportSections() {}
