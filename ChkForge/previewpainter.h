#pragma once
#include <QDialog>
#include <memory>

namespace Ui {
  class PreviewPainter;
}

// TODO: Rename to MinimapPreviewPainter
class PreviewPainter : public QDialog
{
  Q_OBJECT

public:
  explicit PreviewPainter(QWidget *parent = nullptr);
  ~PreviewPainter();

private:
  std::unique_ptr<Ui::PreviewPainter> ui;
};
