#ifndef WIIMOTE_H
#define WIIMOTE_H

#include <opencv2/core/core.hpp>
#include <QThread>
#include <vector>
#include "wiiuse.h"

#include "manipulator.h"

vector<Wiimote> wiimotes;

#define MAX_wii_motes 2

class WiiMote {
public:
//protected:
  wiimote_t* t;
  WiiMote(wiimote_t* wm) : t(wm) {}

//public:
  virtual cv::Point3f getAccel();
  virtual bool button(int ix);
};

class WiiManager {
public:
//protected:
  wiimote_t** wii_mote_ts;
  vector<WiiMote> wii_motes;
//public:  
  WiiManager();
  ~WiiManager();
  unsigned search();
  bool poll();
};

#endif
