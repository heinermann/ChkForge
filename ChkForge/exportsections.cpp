#include "exportsections.h"
#include "ui_exportsections.h"

ExportSections::ExportSections(QWidget *parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::ExportSections>())
{
  ui->setupUi(this);
}

ExportSections::~ExportSections() {}
