#pragma once
#include <QDialog>
#include <memory>

namespace Ui {
  class ExportSections;
}

class ExportSections : public QDialog
{
  Q_OBJECT

public:
  explicit ExportSections(QWidget *parent = nullptr);
  ~ExportSections();

private:
  std::unique_ptr<Ui::ExportSections> ui;
};
