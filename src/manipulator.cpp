#include "manipulator.h"

cv::Point3f Manipulator :: getAccel() { accel->getAccel(); }
cv::Point3f Manipulator :: getPos()   { pos  ->getPos();   }

bool Manipulator :: pressed(int ix) { return button->pressed(ix);   }
int  Manipulator :: buttonCount()   { return button->buttonCount(); }
