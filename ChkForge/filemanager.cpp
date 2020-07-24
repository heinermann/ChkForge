#include "filemanager.h"
#include "ui_filemanager.h"

FileManager::FileManager(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FileManager)
{
  ui->setupUi(this);
}

FileManager::~FileManager()
{
  delete ui;
}
