#include "app_controller.h"

using namespace std;

int main(int argc, char *argv[]) {
  // The application constructor initializes the app.
  AppController app(argc, argv);
  return app.run();
}
