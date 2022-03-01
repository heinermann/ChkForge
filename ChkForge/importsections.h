#pragma once
#include <QDialog>
#include <memory>

namespace Ui {
  class ImportSections;
}

class ImportSections : public QDialog
{
  Q_OBJECT

public:
  explicit ImportSections(QWidget *parent = nullptr);
  ~ImportSections();

private:
  std::unique_ptr<Ui::ImportSections> ui;
};
