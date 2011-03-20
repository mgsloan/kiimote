#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include <opencv2/core/core.hpp>

class IAccel  { virtual cv::Point3f getAccel() = 0; };
class IPos    { virtual cv::Point3f getPos() = 0; };

class IButton {
  virtual bool pressed(int ix) = 0;
  virtual int buttonCount() = 0;
};

// 6-DOF manipulator with button info. Forwards to underlying
// implementation.
class Manipulator : public IAccel, public IPos, public IButton {
  IAccel* accel;
  IPos* pos;
  IButton* button;

  Manipulator(IAccel* a, IPos* p, IButton* b);

  virtual cv::Point3f getAccel();
  virtual cv::Point3f getPos();
  virtual bool pressed(int ix);
  virtual int buttonCount();
};

#endif
