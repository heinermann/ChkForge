#include "appsettings.h"

#include "ui_appsettings.h"
#include "language.h"

#include <QLocale>
#include <tuple>

AppSettings::AppSettings(QWidget* parent)
  : QDialog(parent)
  , ui(std::make_unique<Ui::AppSettings>())
{
  ui->setupUi(this);

  populateLanguages();
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

void AppSettings::populateLanguages() {
  QLocale app_lang = ChkForge::GetLanguage();
  const QLocale* best_locale;

  const auto locale_eq = [&](QLocale loc) {
    return loc.language() == app_lang.language() && loc.country() == app_lang.country();
  };
  const auto lang_only_eq = [&](QLocale loc) {
    return loc.language() == app_lang.language();
  };

  best_locale = std::ranges::find_if(SUPPORTED_LOCALES, locale_eq);
  if (best_locale == nullptr) {
    best_locale = std::ranges::find_if(SUPPORTED_LOCALES, lang_only_eq);
    if (best_locale == nullptr) {
      best_locale = &SUPPORTED_LOCALES[0];
    }
  }

  for (auto& locale : SUPPORTED_LOCALES) {
    QString text = locale.nativeLanguageName();
    QIcon icon = QIcon(":/flag/" + QLocale::countryToCode(locale.country()));

    ui->cmb_language->addItem(icon, text, locale);
  }

  ui->cmb_language->setCurrentIndex(best_locale - SUPPORTED_LOCALES);
}

QLocale AppSettings::language() {
  return ui->cmb_language->currentData().toLocale();
}
