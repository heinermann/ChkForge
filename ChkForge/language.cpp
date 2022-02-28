#include "language.h"

#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

namespace ChkForge {
  QLocale AppLanguage = QLocale();

  void LoadTranslationPath(QTranslator& translator, const QString& path) {
    if (translator.load(path)) {
      QApplication::installTranslator(&translator);
    }
    else {
      QMessageBox::warning(nullptr, nullptr, "Failed to load translation file: " + path + ".qm");
    }
  }

  void SetLanguage(QLocale language) {
    static QTranslator qt_lang;
    static QTranslator editor_lang;

    QApplication::removeTranslator(&qt_lang);
    QApplication::removeTranslator(&editor_lang);

    QString country_code = QLocale::countryToCode(language.country()).toLower();
    QString language_code = QLocale::languageToCode(language.language()).toLower();

    LoadTranslationPath(qt_lang, "translations/qt_" + language_code);
    LoadTranslationPath(editor_lang, "translations/chkforge_" + language_code + "_" + country_code);

    AppLanguage = language;
  }

  QLocale GetLanguage() {
    return AppLanguage;
  }

}
