#ifndef WIIMOTE_H
#define WIIMOTE_H

#include <opencv2/core/core.hpp>
#include <QThread>
#include <vector>
#include "wiiuse.h"

#include "manipulator.h"

class WiiMote {
public:
//protected:
  wiimote_t* wm;
  WiiMote(wiimote_t* p) : wm(p) {}

//public:
  virtual cv::Point3f getAccel();
  virtual bool button(unsigned ix);
};

#define MAX_wii_motes 2

class WiiManager {
public:
//protected:
  wiimote_t** wii_mote_ts;
  std::vector<WiiMote> wii_motes;
//public:  
  WiiManager();
  ~WiiManager();
  unsigned search();
  bool poll();
};

#endif
