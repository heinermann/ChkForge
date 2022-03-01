#include "filemanager.h"
#include "ui_filemanager.h"

FileManager::FileManager(QWidget *parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::FileManager>())
{
  ui->setupUi(this);
}

FileManager::~FileManager() {}
