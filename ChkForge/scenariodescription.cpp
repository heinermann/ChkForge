#include "scenariodescription.h"
#include "ui_scenariodescription.h"

#include <QDialog>
#include <QAction>
#include <QMessageBox>

#include "charmap.h"

ScenarioDescription::ScenarioDescription(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::ScenarioDescription)
{
  ui->setupUi(this);
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

void ScenarioDescription::accept() {
  name = ui->txt_name->toPlainText();
  description = ui->txt_description->toPlainText();
  
  name.replace("\n", "\r");
  description.replace("\n", "\r");

  QDialog::accept();
}

void ScenarioDescription::on_toolButton_pressed() {
  //auto charmap = CharMap(this, ui->txt_name->font());
  auto charmap = CharMap(this, QFont{"EurostileExtReg"});
  charmap.exec();
}