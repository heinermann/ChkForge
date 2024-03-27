#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QTime>
#include <QFileDialog>
#include <QDir>
#include <QFontDatabase>
#include <QTemporaryFile>
#include <QStyleHints>
#include <QSettings>

#include "../openbw/bwglobal.h"
#include "../openbw/bwglobal_ui.h"
#include "../openbw/openbw/ui/common.h"

#include "icons.h"
#include "Utils.h"

#include "../Chkdraft/CrossCutLib/Logger.h"

Logger logger;

namespace bwgame {
  global_state global_st;
  global_ui_state global_ui_st;

  namespace ui {
    void log_str(a_string str) {
      static FILE* log_file = nullptr;

      fwrite(str.data(), str.size(), 1, stdout);
      fflush(stdout);
      if (!log_file) log_file = fopen("log.txt", "wb");
      if (log_file) {
        fwrite(str.data(), str.size(), 1, log_file);
        fflush(log_file);
      }
    }

    void fatal_error_str(a_string str) {
      log("fatal error: %s\n", str);
      //std::terminate();
    }
  }
}

bool init_bwgame(const QString& starcraft_dir) {
  std::string install_dir = toStdString(starcraft_dir);

  try {
    auto load_data_file = bwgame::data_loading::data_files_directory(install_dir);

    bwgame::global_st.init(load_data_file);

    bwgame::global_ui_st.global_volume = 0;
    bwgame::global_ui_st.load_data_file = [&](bwgame::a_vector<uint8_t>& data, bwgame::a_string filename) {
      load_data_file(data, std::move(filename));
    };
    bwgame::global_ui_st.init(load_data_file);

    auto load_font = [&](const char* font_path) {
      std::vector<uint8_t> font_data;
      load_data_file(font_data, font_path);

      QByteArray fontData{ reinterpret_cast<char*>(font_data.data()), int(font_data.size()) };
      if (QFontDatabase::addApplicationFontFromData(fontData) == -1) {
        QMessageBox::critical(nullptr, QString(), QObject::tr("Failed to load font: %1").arg(font_path));
      }
    };

    load_font("font/bl.ttf"); // BlizzardGlobal
    load_font("font/BLIZZARD-REGULAR.TTF"); // Blizzard
    load_font("font/EUROSTILE-REG.TTF"); // Eurostile
    load_font("font/EUROSTILEEXT-REG.TTF"); // EurostileExtReg
    load_font("font/UDTypos58B_P_H.ttf"); // UDTypos58B-P
    load_font("font/UDTypos510B_P_H.ttf"); // UDTypos510B-P

    //QMessageBox::about(nullptr, "", QFont::substitutes("EurostileExtReg").join(", "));

    return true;
  }
  catch (const std::exception& ex) {
    QMessageBox::critical(nullptr, QString(), QObject::tr("Failed to initialize OpenBW:\n%1\n\nPlease select a different directory.").arg(ex.what()));
  }
  catch (...) {
    QMessageBox::critical(nullptr, QString(), QObject::tr("Unknown error initializing OpenBW. Please select a different directory."));
  }
  return false;
}

void init_bwgame_with_directory() {
  QSettings settings;

  QString found_dir = settings.value("ScPath", "").toString();
  while (found_dir.isEmpty() || !init_bwgame(found_dir)) {
    found_dir = QFileDialog::getExistingDirectory(nullptr, QObject::tr("Find Starcraft Directory"), QDir::rootPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  }
  settings.setValue("ScPath", found_dir);
}

int main(int argc, char *argv[])
{
  QCoreApplication::setApplicationName("ChkForge");
  QCoreApplication::setOrganizationName("StareditMemes");

  QApplication app(argc, argv);

  QFont appFont = app.font();
  appFont.setPixelSize(11);
  app.setFont(appFont);

  init_bwgame_with_directory();
  ChkForge::Icons::init();

  MainWindow w;
  w.showMaximized();

  return app.exec();
}
