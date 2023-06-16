#include "language.h"

#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

namespace ChkForge {
  QLocale AppLanguage = QLocale();

  bool LoadTranslationPath(QTranslator& translator, const QString& path, bool ignoreError = false) {
    bool result = translator.load(path);
    if (result) {
      QApplication::installTranslator(&translator);
    }
    else if (!ignoreError) {
      QMessageBox::warning(nullptr, nullptr, "Failed to load translation file: " + path + ".qm");
    }
    return result;
  }

  void SetLanguage(QLocale language) {
    static QTranslator qt_lang;
    static QTranslator editor_lang;

    QApplication::removeTranslator(&qt_lang);
    QApplication::removeTranslator(&editor_lang);

    QString country_code = QLocale::countryToCode(language.country()).toLower();
    QString language_code = QLocale::languageToCode(language.language()).toLower();

    if (!LoadTranslationPath(qt_lang, "translations/qt_" + language_code, true)) {
      LoadTranslationPath(qt_lang, "translations/qt_" + language_code + "_" + country_code);
    }
    LoadTranslationPath(editor_lang, "translations/chkforge_" + language_code + "_" + country_code);

    AppLanguage = language;
  }

  QLocale GetLanguage() {
    return AppLanguage;
  }

}
