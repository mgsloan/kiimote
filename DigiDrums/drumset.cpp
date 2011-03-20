#define GLEW_STATIC
#include <GL/glew.h>

#include "drumset.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "wiiuse.h"
#include <algorithm>
#include <QVector2D>
#include <limits.h>

#include "utils.h"

using namespace std;

Drum::Drum(cv::Point3f _location, cv::Point3f _orientation, double _radius, int _sound) :
  location(_location), orientation(_orientation), radius(_radius), sound(_sound) {
}

void Drum::draw(cv::Point3f color) {
  glColor3f(color.x, color.y, color.z);

  const int segments = 20;
  const double height = .1;
  const double two_pi = M_PI * 2;

  // TODO: use gluQuadrics for this?
//  printf("\nDrawing drum at %f, %f, %f...", location.x, location.y, location.z);
//  fflush(stdout);

  double rad = radius;
  if (color.z != 1) rad *= 1.25;

  glPushMatrix();
    glTranslatef(location.x, location.y, location.z);
    if (orientation.z != 0) {
        float ang = atan(orientation.y / orientation.z);
        glRotatef(ang * 180.0f / M_PI, 1, 0, 0);
    }
    for (int level = -1; level <= 1; level += 2) {
      glBegin(GL_TRIANGLE_FAN);
        glNormal3d(0, 0, level);
        glVertex3d(0, 0, level * height);
        for (int segment = 0; segment <= segments; segment++) {
          double radians = segment * two_pi / segments;
          glNormal3d(0, level, 0);
          glVertex3d(cos(radians) * rad, level * height, sin(radians) * rad);
        }
      glEnd();
    }
    glBegin(GL_TRIANGLE_STRIP);
      for (int segment = 0; segment <= segments; segment++) {
        for (int level = -1; level <= 1; level += 2) {
          double radians = segment * two_pi / segments;
          glNormal3d(cos(radians) * rad, 0, sin(radians) * rad);
          glVertex3d(cos(radians) * rad, level * height, 
                     sin(radians) * rad);
        }
      }
    glEnd();
  glPopMatrix();
}

DrumSet::DrumSet() : instrument(0) {}

void DrumSet::addDrum(Drum* drum) {
  drums.push_back(drum);
}

void DrumSet::setInstrument(int _instrument) {
  instrument = _instrument;
}

void DrumSet::draw() {
  // Draw the drums
  for (int index = 0; index < drums.size(); index++)
    drums[index]->draw(cv::Point3f(1, 1, 1));
  for (int stick = 0; stick < sticks.size(); stick++) {
    int ix = sticks[stick].drum;
    if (ix != -1 && ix < drums.size()) drums[ix]->draw(sticks[stick].color);
  }

  /*
  for (int index = 0; index < drums.size(); index++) {
    Drum* drum = drums[index];
    int stick;
    for (stick = 0; stick < sticks.size(); stick++) {
      if (sticks[stick].drum == index) {
        printf("\nDrum %d, stick %d", index, stick);
        break;
      }
    }
    if (stick == sticks.size()) {
      stick = -1;
    }
    drum->draw(stick);
  } */

  // Draw the drum sticks
  for (int i = 0; i < sticks.size(); i++) {
    //if (sticks[i].visible) {
      cv::Point3f& pos = sticks[i].pos;
      /*if (i == 0) {
        glColor3d(1, 0, 0);
      } else if (i == 1) {
        glColor3d(0, 1, 0);
      } */
      glColor3d(sticks[i].color.x,sticks[i].color.y,sticks[i].color.z);

/*
      GLUquadric* q = gluNewQuadric();
      glPushMatrix();
        glTranslatef(pos.x,pos.y,pos.z);
        gluSphere(q, .1, 10, 10);
      glPopMatrix();
      gluDeleteQuadric(q);
 */

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
   // }
  }
}

DrumSet::~DrumSet() {
  for (vector<Drum*>::iterator iter = drums.begin();
      iter != drums.end(); iter++) {
    delete *iter;
  }
}

int DrumSet::intersects(const cv::Point3f point) {
  // Determine closest drum...
  /*
  double min_radius = 0;
  int closest_drum = -1;
  int index;
  QVector2D projected_point(point.x, point.z);
  for (index = 0; index < drums.size(); index++) {
    Drum* drum = drums[index];
    QVector2D projected_location(drum->location.x, drum->location.z);
    double radius = (projected_location - projected_point).length() - drum->radius;
    if (closest_drum == -1 || radius < min_radius) {
      closest_drum = index;
      min_radius = radius;
    }
  }

  return (closest_drum > -1 && min_radius < radius_threshold) ?
    closest_drum : -1;
  */

  double min_dist = numeric_limits<double>::infinity();
  int closest_drum = -1;
  for (int i = 0; i < drums.size(); i++) {
    double dist = mag(drums[i]->location - point);
    if (dist < min_dist) {
      min_dist = dist;
      closest_drum = i;
    }
  }
  return min_dist < radius_threshold ? closest_drum : -1;
}

DrumSet::DrumStick::DrumStick() : 
  pos(0,0,0), orient(0,0,0), color(1,0,0), 
  visible(false), drum(-1) {}

DrumSet::DrumStick::DrumStick(int _drum) : 
  pos(0,0,0), orient(0,0,0), color(1,0,0), 
  visible(false), drum(_drum) {}

cv::Point3f DrumSet::DrumStick::getTip() {
  cv::Point3f ret = ret;
  ret *= -2.0 / mag(ret);
  ret.z = -ret.z;
  ret += pos;
  return ret;
}

void DrumSet::initSticks(int num_sticks) {
  for (int i = 0; i < num_sticks; i++) {
    sticks.push_back(DrumSet::DrumStick());
  }
  // TODO: make sure there is a drum #4.
  sticks.push_back(DrumSet::DrumStick(4));
}


