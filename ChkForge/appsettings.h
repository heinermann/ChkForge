#pragma once

#include <QDialog>
#include <memory>

namespace Ui {
  class AppSettings;
}

class AppSettings : public QDialog
{
  Q_OBJECT

public:
  explicit AppSettings(QWidget* parent = nullptr);
  ~AppSettings();

  QLocale language();

private:
  std::unique_ptr<Ui::AppSettings> ui;

  void populateLanguages();
};
