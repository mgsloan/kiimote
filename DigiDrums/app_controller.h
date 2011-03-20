#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#ifdef OPENAL_SOUND_PLAYER
#include <AL/al.h>
#include <AL/alut.h>
#endif

#include <QThread>
#include <QApplication>
#include "drum_viewer.h"
#include "app_viewer.h"
#include "kw_controller.h"
#include "drum_controller.h"
#include <string>
#include <QTimer>

#define USE_KINECT_WII

class AppController : QApplication {
  Q_OBJECT
protected:
  QTimer timer;
  std::string dir_name;
  AppViewer window;
  KinectController* kcp;

public:
  bool no_kinect;

  AppController(int argc, char* argv[]);
  int run();
  ~AppController();

protected slots:
  /** Called by the timer to update stuff. */
  void update();
};

#endif // APP_CONTROLLER_H
