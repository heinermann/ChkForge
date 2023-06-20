#include "win32.h"

int main(int argc, char* argv[]) {
  ShowWindow(GetConsoleWindow(), SW_HIDE);

  return 0;
}
