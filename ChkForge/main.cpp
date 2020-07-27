#include "mainwindow.h"

#include <QApplication>
#include <SDL.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  SDL_Init(SDL_INIT_VIDEO);

  MainWindow w;
  w.show();
  w.afterCreated();
  int result = a.exec();

  SDL_Quit();

  return result;
}
