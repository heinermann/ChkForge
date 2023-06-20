// MEMES
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef WIN32
  ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

  return 0;
}
