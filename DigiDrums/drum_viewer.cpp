#define GLEW_STATIC
#include <GL/glew.h>

#include "drum_viewer.h"
#include "clone_record.h"

#include "wiiuse.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <ntk/mesh/mesh.h>

using namespace std;
using namespace cv;

//Lighting Variables///////////////////////////////////////////////////////////
const cv::Point3f DrumViewer::up_direction(0,1,0);
const double DrumViewer::fov = 90;
const double DrumViewer::z_clip_near = .1;
const double DrumViewer::z_clip_far = 100;

const GLfloat DrumViewer::light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
const GLfloat DrumViewer::light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
const GLfloat DrumViewer::light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
const GLfloat DrumViewer::light_position[] = { 1.0, 1.0, 1.0, 0.0 };

const GLfloat DrumViewer::mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
const GLfloat DrumViewer::mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
const GLfloat DrumViewer::mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
const GLfloat DrumViewer::high_shininess[] = { 100.0 };

DrumViewer::DrumViewer(DrumSet* _drum_set, IMouseInput* _mouse_input) :
  drum_set(_drum_set),
  mouse_input(_mouse_input),
  kinect_controller(NULL),
  eye_position(0,0,5),
  look_at(0,0,-10),
  samples_a(0),
  samples_b(0),
  color_a(0,0,0),
  color_b(0,0,0) {
  mouse_input->setMouseListener(this);
}

void DrumViewer::addListener(IDrumstickListener* listener) {
  drum_listeners.push_back(listener);
}


void DrumViewer::mouseMoved(int x, int y) {
  last_mouse_pos = QVector2D(x, y);
  cv::Point3f pos;
  if (translateMouse(last_mouse_pos, pos)) {
    for (int di = 0; di < drum_listeners.size(); di++) {
      drum_listeners[di]->onStickMove(0, pos, cv::Point3f(0,0,0));
    }
  }
}

void DrumViewer::mousePressEvent(int button) {
  for (int di = 0; di < drum_listeners.size(); di++) {
    drum_listeners[di]->onStickHit(0, 0.9);
  }
}

void DrumViewer::mouseLeft() {
  for (int di = 0; di < drum_listeners.size(); di++) {
    drum_listeners[di]->onStickVanish(0);
  }
}

int DrumViewer::countSticks() { return 1; }

GLfloat flip_mat[] = {-1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1};

void draw_axis( float scale )
{
  glPushMatrix();
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glScalef( scale, scale, scale );

  glBegin( GL_LINES );

  glColor3f( 1.0, 0.0, 0.0 );
  glVertex3f( .8f, 0.05f, 0.0 );  glVertex3f( 1.0, 0.25f, 0.0 ); // Letter X
  glVertex3f( 0.8f, .25f, 0.0 );  glVertex3f( 1.0, 0.05f, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 1.0, 0.0, 0.0 );     // X axis

  glColor3f( 0.0, 1.0, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 );	 // Y axis

  glColor3f( 0.0, 0.0, 1.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 );	 // Z axis
  glEnd();
  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glColor3f( 1.0, 1.0, 1.0 );
  glPopMatrix();
}

void draw_points(const ntk::Mesh& mesh, double scale, double alpha) {
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glScalef( scale, scale, scale );
  glPointSize(10.0);
  glPointParameterf(GL_POINT_SIZE_MIN, 1.0);
  glPointParameterf(GL_POINT_SIZE_MAX, 10.0);
  float acoeffs[] = {1.0, -1.0, 0.0};
  glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, acoeffs);

  glBegin(GL_POINTS);
  for (int i = 0; i < mesh.vertices.size(); ++i)
  {
    const cv::Point3f& v = mesh.vertices[i];
    if (mesh.hasColors())
      glColor4f(mesh.colors[i][0]/250.0, mesh.colors[i][1]/250.0, mesh.colors[i][2]/250.0, alpha);
    glVertex3f(v.x, v.y, v.z);
  }
  glEnd();
  glPopMatrix();
}

void DrawAxis( float scale )
{
  glPushMatrix();
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glScalef( scale, scale, scale );

  glBegin( GL_LINES );

  glColor3f( 1.0, 0.0, 0.0 );
  glVertex3f( .8f, 0.05f, 0.0 );  glVertex3f( 1.0, 0.25f, 0.0 ); // Letter X
  glVertex3f( 0.8f, .25f, 0.0 );  glVertex3f( 1.0, 0.05f, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 1.0, 0.0, 0.0 );     // X axis

  glColor3f( 0.0, 1.0, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 );	 // Y axis

  glColor3f( 0.0, 0.0, 1.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 );	 // Z axis
  glEnd();
  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glColor3f( 1.0, 1.0, 1.0 );
  glPopMatrix();
}

void DrumViewer::paintGL() {
  // TODO: render text on the surface.
  //QPainter painter;
  //painter.begin(this);
  //QPen textPen(Qt::white);
  //glColor4d(1,1,1,1);
  //QFont textFont;
  //textFont.setPixelSize(50);
  //renderText(0, 0, 0, "DigiDrums", textFont);
  //painter.drawText(QRect(0,0,400,40), Qt::AlignLeft, "DigiDrums");
  //painter.end();

  // Set up lighting
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  // Set up material
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

  // Enable lights
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // Enable depth testing
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  // Enable other features
  //glEnable(GL_NORMALIZE); // TODO: needed?
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_BLEND);
  glEnable(GL_POLYGON_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // TODO: change?

  // Set up the projection matrix
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  QSize size = screen_manager->getScreenSize();
  gluPerspective(fov,(double)size.width()/(double)size.height(),
    z_clip_near, z_clip_far);

  // Set up the modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixf(flip_mat); // TODO: replace with glScale(-1,0,0)
  gluLookAt(eye_position.x, eye_position.y, eye_position.z,
    look_at.x, look_at.y, look_at.z,
    up_direction.x, up_direction.y, up_direction.z);

  // Copy matrix parameters for determining mouse rays.
  glGetDoublev(GL_PROJECTION_MATRIX, proj_matrix);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  glGetIntegerv(GL_VIEWPORT, viewport);
  mouse_enabled = true;

  // Clear things out
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(.5, .5, .5, 0.0);
  glClearAccum(0.0, 0.0, 0.0, 0.0);

  // Draw axis
  draw_axis(1.0f);

  // Draw drum set
  drum_set->draw();

  // Draw the Kinect mesh
  if (kinect_controller) {
    ntk::Mesh& mesh = kinect_controller->getMesh(),
               bgmesh = kinect_controller->getBgMesh();
    mesh_transform->apply(mesh); 
    bg_transform->apply(bgmesh); 
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    draw_points(mesh, 10, 1);
    draw_points(bgmesh, 10, 1);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    if (kinect_controller->calibrating) {
      cv::Vec3b col = kinect_controller->red_profile.mean;
      cv::Point3f pos = kinect_controller->calib_rpos;
      
      double r = (double)col[0] / 255.0, 
             g = (double)col[1] / 255.0, 
             b = (double)col[2] / 255.0;
      //printf("%f %f %f\n", r, g, b);
      glColor4d(r, g, b, 0.5);
      GLUquadric* q = gluNewQuadric();
      glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        gluSphere(q, 0.3, 10, 10);
      glPopMatrix();
      gluDeleteQuadric(q);

      col = kinect_controller->green_profile.mean;
      pos = kinect_controller->calib_gpos;

      r = (double)col[0] / 255.0;
      g = (double)col[1] / 255.0;
      b = (double)col[2] / 255.0;
      glColor4d(r, g, b, 0.5);
      GLUquadric* q2 = gluNewQuadric();
      glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        gluSphere(q2, 0.3, 10, 10);
      glPopMatrix();
      gluDeleteQuadric(q2);
    }
  }

  if (drum_controller) {
    vector<PlaybackThread*>& clones = drum_controller->clones;
    for (int i = 0; i < clones.size(); i++) {
      draw_points(clones[i]->getMesh(), 10, 0.5);
    }
  }

  // Restore OpenGL
  // TODO: restore other state parameters
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

bool DrumViewer::getPosition(int stick, cv::Point3f& result) {
  return translateMouse(last_mouse_pos, result);
}

void DrumViewer::bindScreenManager(IScreenManager* manager) {
  screen_manager = manager;
}

bool DrumViewer::getOrientation(int stick, cv::Point3f& result) {
  return false;
}

bool DrumViewer::getColor(int stick, cv::Point3f& result) {
  return false;
}

void DrumViewer::vibrateStick(int stick) {}

bool DrumViewer::translateMouse(QVector2D pos, Point3f& result) {
  // Return false if the projection matrix hasn't been detected yet.
  if (!mouse_enabled) { return false; }
  QSize size = screen_manager->getScreenSize();
  int height = size.height(), width = size.width();
  double half_width = width / 2.0, half_height = height / 2.0;
  QVector2D normalized_position((pos.x() - half_width) / half_width,
    (pos.y() - half_height) / half_height);
  printf("\nMouse is %f, %f", normalized_position.x(), normalized_position.y());

  GLdouble results[6];
  gluUnProject(pos.x(), height-pos.y(), 0, modelview_matrix, proj_matrix,
    viewport, results, results + 1, results + 2);
  gluUnProject(pos.x(), height-pos.y(), 1, modelview_matrix, proj_matrix,
    viewport, results + 3, results + 4, results + 5);
  cv::Point3f mouse_ray(results[3] - results[0], results[4] - results[1],
    results[5] - results[2]);

  printf("\nMouse ray is %f, %f, %f", mouse_ray.x, mouse_ray.y, mouse_ray.z);
  //result = Point3f(0,0,0);
  //return true;

  printf("\nEye is %f, %f, %f", eye_position.x, eye_position.y, eye_position.z);

  // Make sure the mouse hits the plane!
  if ((mouse_ray.y < 0 && eye_position.y < 0) ||
      (mouse_ray.y > 0 && eye_position.y > 0)) {
    return false;
  }

  // Now, find intersection
  double t = eye_position.y / -mouse_ray.y;
  result = t * mouse_ray + eye_position;
  printf("\nResult is %f, %f, %f", result.x, result.y, result.z);
  return true;
}

void DrumViewer::setKinect( KinectController* controller) { kinect_controller = controller; }

void DrumViewer::setDrumSetController( DrumSetController* cont) {
  drum_controller = cont;
}
