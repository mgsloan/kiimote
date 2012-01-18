#define GLEW_STATIC
#include <GL/glew.h>

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <ntk/mesh/mesh.h>

using namespace std;
using namespace cv;

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

Viewer::Viewer(App* a)
  : app(a)
  , eye_position(0, 0, 5)
  , look_at(0,0,-10)
  , color_a(0,0,0)
  , color_b(0,0,0)
  {}

void drawSphere(cv::Vec3b col, cv::Point3f pos, double size, int subs) {
  r = (double)col[0] / 255.0;
  g = (double)col[1] / 255.0;
  b = (double)col[2] / 255.0;
  glColor4d(r, g, b, 0.5);
  GLUquadric* q2 = gluNewQuadric();
  glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    gluSphere(q2, size, subs, subs);
  glPopMatrix();
  gluDeleteQuadric(q2);
}

void Viewer::paintGL() {
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
  if (app->kinect) {
    ntk::Mesh& mesh = app->kinect->generated_mesh,
               bgmesh = kinect_controller->getBgMesh();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    draw_points(mesh, 10, 1);
    draw_points(bgmesh, 10, 1);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    if (app->kinect->do_calibration) {
      drawSphere( app->kinect->red_profile.mean
                , app->kinect->calib_rpos
                , 0.3, 10);

      drawSphere( app->kinect->green_profile.mean
                , app->kinect->calib_gpos
                , 0.3, 10);
    }
  }

  // Draw manipulators.
  app->red.draw();
  app->green.draw();

  // Restore OpenGL
  // TODO: restore other state parameters
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}