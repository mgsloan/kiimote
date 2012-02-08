#include "app.h"


using namespace std;

// Command-line options
namespace opt
{
  ntk::arg<bool> no_kinect("--no_kinect", "Do not depend on kinect input", 0);
}

App::App(int argc, char* argv[])
  : QApplication(argc, argv)
  , window(this)
  , kinect(NULL)
  , red(true, 0), green(false, 1) {
  QApplication::setGraphicsSystem("raster");

  // Parse command-line arguments
  ntk::arg_parse(argc, argv);
  no_kinect = opt::no_kinect();
}

int App::run() {
  // Initialize OpenGL
  GLenum err = glewInit();
  if (GLEW_OK == err) {
    printf("Got glew!!\n");
  }

  if (!no_kinect) {
    kinect = new Kinect("../../kinect_calibration.yml");
  }

  no_wii = !wii.search();

  glViewport(0,0,640,480);
  window.show();

  // Start the timer
  connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
  timer.start(1000 / 30);

  // Blocks until app exit.
  return exec();
}

void App::update() {
  if (!no_wii && wii.poll()) {
    red.update(this);
    green.update(this);
  }
  window.update();
}
