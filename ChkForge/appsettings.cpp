#include "appsettings.h"

#include "ui_appsettings.h"
#include "language.h"

#include <QLocale>
#include <QStyle>
#include <QStyleFactory>
#include <QSettings>
#include <tuple>

AppSettings::AppSettings(QWidget* parent, const QSettings &defaultValues)
  : QDialog(parent)
  , ui(std::make_unique<Ui::AppSettings>())
  , defaults(defaultValues)
{
  ui->setupUi(this);

  style = defaults.value("theme", "fusion").toString();
  language = defaults.value("language", findSupportedLocale(QLocale())).toLocale();

  populateLanguages();
  populateThemes();
  settingUp = false;
}

AppSettings::~AppSettings() {}

namespace {
  const QLocale SUPPORTED_LOCALES[] = {
    QLocale(QLocale::English, QLocale::Canada),
    QLocale(QLocale::English, QLocale::UnitedStates),
    QLocale(QLocale::German, QLocale::Germany),
    QLocale(QLocale::Spanish, QLocale::Spain),
    //QLocale(QLocale::Spanish, QLocale::Mexico),
    QLocale(QLocale::French, QLocale::France),
    QLocale(QLocale::Italian, QLocale::Italy),
    QLocale(QLocale::Japanese, QLocale::Japan),
    QLocale(QLocale::Korean, QLocale::RepublicOfKorea),
    QLocale(QLocale::Polish, QLocale::Poland),
    QLocale(QLocale::Portuguese, QLocale::Brazil),
    QLocale(QLocale::Chinese, QLocale::China),
    QLocale(QLocale::Chinese, QLocale::Taiwan)
  };
}

QLocale AppSettings::findSupportedLocale(QLocale target) {
  const QLocale* result;

  const auto locale_eq = [&](QLocale loc) { return loc.language() == target.language() && loc.country() == target.country(); };
  const auto lang_eq = [&](QLocale loc) { return loc.language() == target.language(); };

  result = std::ranges::find_if(SUPPORTED_LOCALES, locale_eq);
  if (result != nullptr) return *result;

  result = std::ranges::find_if(SUPPORTED_LOCALES, lang_eq);
  if (result != nullptr) return *result;

  return SUPPORTED_LOCALES[0];
}

void AppSettings::populateLanguages() {
  QLocale best_locale = findSupportedLocale(language);

  for (auto& locale : SUPPORTED_LOCALES) {
    QString text = locale.nativeLanguageName();
    QIcon icon = QIcon(":/flag/" + QLocale::countryToCode(locale.country()));

    ui->cmb_language->addItem(icon, text, locale);
    if (locale == best_locale) {
      ui->cmb_language->setCurrentText(text);
    }
  }
}

void AppSettings::populateThemes() {
  ui->cmb_theme->addItems(QStyleFactory::keys());
  ui->cmb_theme->setCurrentText(style);
}

void AppSettings::on_cmb_theme_currentTextChanged(const QString& text) {
  if (settingUp) return;
  
  style = text;
  QApplication::setStyle(style);
}

void AppSettings::on_cmb_language_currentIndexChanged(int index) {
  if (settingUp) return;
  
  language = ui->cmb_language->currentData().toLocale();
  ChkForge::SetLanguage(language);
  ui->retranslateUi(this);
}
