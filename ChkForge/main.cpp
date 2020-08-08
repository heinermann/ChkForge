#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QTime>

#include "../openbw/bwglobal.h"
#include "../openbw/bwglobal_ui.h"
#include "../openbw/openbw/ui/common.h"

#include "icons.h"

#include <CommanderLib/Logger.h>

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

void init_bwgame() {
  auto load_data_file = bwgame::data_loading::data_files_directory("C:/Program Files (x86)/StarCraft 1.16.1");

  bwgame::global_st.init(load_data_file);

  bwgame::global_ui_st.global_volume = 0;
  bwgame::global_ui_st.load_data_file = [&](bwgame::a_vector<uint8_t>& data, bwgame::a_string filename) {
    load_data_file(data, std::move(filename));
  };
  bwgame::global_ui_st.init(load_data_file);
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  auto clock = QTime();
  clock.start();

  init_bwgame();
  ChkForge::Icons::init();

  auto elapsed = clock.elapsed();

  MainWindow w;
  w.showMaximized();

  return a.exec();
}
