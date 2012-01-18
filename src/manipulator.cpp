#include "manipulator.h"

Manipulator::Manipulator(bool red, int wii) : isRed(red), wiiIx(wii) {
  color = isRed ? cv::Point3f(1,0,0) : cv::Point3f(0,1,0);
}

void Manipulator::draw() {
  glColor3d(color.x, color.y, color.z);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
      glLineWidth(4.0);
      glBegin( GL_LINES );
      glVertex3f( pos.x, pos.y, pos.z );
      cv::Point3f tip = sticks[i].getTip();
      glVertex3f( tip.x, tip.y, tip.z );
      glEnd();
      glLineWidth(1.0);
  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
}

void Manipulator::update(App* app) {
  if (wiiIx < app->wii.wii_motes.size())
    orient = app->wii.wii_motes[wiiIx];

  if (app->kinect)
    pos = isRed ? app->kinect->red  ->computeValue()
                : app->kinect->green->computeValue();
}

cv::Point3f Manipulator::getTip() {
  cv::Point3f ret = ret;
  ret *= -2.0 / mag(ret);
  ret.z = -ret.z;
  ret += pos;
  return ret;
}