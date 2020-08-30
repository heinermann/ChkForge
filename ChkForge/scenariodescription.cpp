#include "scenariodescription.h"
#include "ui_scenariodescription.h"

#include <QDialog>

ScenarioDescription::ScenarioDescription(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::ScenarioDescription)
{
  ui->setupUi(this);

  ui->txt_name->setAlignment(Qt::AlignCenter);
}

ScenarioDescription::~ScenarioDescription() {
  delete ui;
}

int ScenarioDescription::exec() {
  ui->txt_name->setPlainText(name);
  ui->txt_name->setAlignment(Qt::AlignCenter);
  ui->txt_description->setPlainText(description);

  return QDialog::exec();
}
