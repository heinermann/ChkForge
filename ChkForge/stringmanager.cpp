#include "stringmanager.h"
#include "ui_stringmanager.h"

StringManager::StringManager(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::StringManager)
{
  ui->setupUi(this);
}

StringManager::~StringManager()
{
  delete ui;
}
