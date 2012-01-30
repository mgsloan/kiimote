#ifndef APP_H
#define APP_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "viewer.h"
#include "kinect.h"
#include "wiimote.h"
#include "manipulator.h"

#include <QTimer>
#include <string>

class App : QApplication {
  Q_OBJECT
public:
//protected:
  QTimer timer;
  Viewer window;
  Kinect* kinect;
  WiiManager wii;
  Manipulator red, green;

  bool no_kinect;

//public:
  App(int argc, char* argv[]);

  int run();

protected slots:
  /** Called by the timer to update stuff. */
  void update();
};

#endif // APP_H