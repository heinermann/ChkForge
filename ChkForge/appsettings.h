#pragma once

#include <QDialog>
#include <QSettings>
#include <QLocale>
#include <memory>

namespace Ui {
  class AppSettings;
}

class AppSettings : public QDialog
{
  Q_OBJECT

public:
  explicit AppSettings(QWidget* parent, const QSettings &defaults);
  ~AppSettings();

  QLocale language;
  QString style;

  static QLocale findSupportedLocale(QLocale target);

private:
  std::unique_ptr<Ui::AppSettings> ui;

  void populateLanguages();
  void populateThemes();
  
  bool settingUp = true;
  const QSettings& defaults;

private slots:
  void on_cmb_theme_currentTextChanged(const QString& text);
  void on_cmb_language_currentIndexChanged(int index);
};
