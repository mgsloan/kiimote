#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include <opencv2/core/core.hpp>

// Forward declaration of App for pointer.
class App;

class Manipulator {
public:
//protected:
  bool isRed;
  int wiiIx;

  cv::Point3f pos;
  cv::Point3f orient;
  cv::Point3f color;

//public:
  Manipulator(bool red, int wii);

  void draw();
  void update(App* app);
  cv::Point3f getTip();
};

#endif //MANIPULATOR_H