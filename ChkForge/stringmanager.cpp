#include "stringmanager.h"
#include "ui_stringmanager.h"

StringManager::StringManager(QWidget *parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::StringManager>())
{
  ui->setupUi(this);
}

StringManager::~StringManager() {}
