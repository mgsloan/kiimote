#ifndef WIIMOTE_H
#define WIIMOTE_H

#include <opencv2/core/core.hpp>
#include <QThread>
#include <vector>
#include "wiiuse.h"

#include "manipulator.h"

vector<Wiimote> wiimotes;

class Wiimote : IAccel, IButton {
  wiimote_t wm;
  virtual cv::Point3f getAccel();
  virtual bool button(int ix);
};

class WiiThread : QThread {
  virtual void run();
};

#endif
