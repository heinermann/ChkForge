#pragma once

#include <QDialog>
#include <memory>

namespace Ui {
  class ExportImage;
}

class ExportImage : public QDialog
{
  Q_OBJECT

public:
  explicit ExportImage(QWidget *parent = nullptr);
  ~ExportImage();

private:
  std::unique_ptr<Ui::ExportImage> ui;
};
