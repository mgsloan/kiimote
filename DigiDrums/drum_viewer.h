#ifndef DRUM_VIEWER_H
#define DRUM_VIEWER_H

#include <stdio.h>
#include <vector>

#include <opencv2/core/core.hpp>

#include <QGLWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QVector2D>

#include "idrumsticks.h"
#include "sound_player.h"
#include "drumset.h"
#include "drum_controller.h"
#include "app_viewer.h"
#include "kinect_controller.h"
#include "mesh_transforms.h"
#include "app_viewer.h"

/**
 * The drum viewer, which displays the drums in the drum kit.
 */
class DrumViewer : public IDrumsticks, public IMouseListener, public IScreenLayer {

public:
  // The drum set to view.
  DrumSet* drum_set;

protected:
  IMouseInput* mouse_input;
  IScreenManager* screen_manager;

  // Drumstick listeners
  vector<IDrumstickListener*> drum_listeners;

public:
  DrumViewer(DrumSet* drum_set, IMouseInput* _mouse_input);

  // IMouseListener
  void mouseMoved(int x, int y);
  void mousePressEvent(int button);
  void mouseLeft();

  // IDrumsticks
  void addListener(IDrumstickListener* listener);
  int countSticks();
  bool getPosition(int stick, cv::Point3f& result);
  bool getOrientation(int stick, cv::Point3f& result);
  bool getColor(int stick, cv::Point3f& result);
  void vibrateStick(int stick);

  // IScreenLayer
  void paintGL();
  void bindScreenManager(IScreenManager* manager);

  // The KinectController to derive mesh data from.
  KinectController* kinect_controller;
  void setKinect( KinectController* controller );

  // The DrumController, in order to get the clones to draw.
  DrumSetController* drum_controller;
  void setDrumSetController(DrumSetController* cont);

  cv::Point3f eye_position;
  cv::Point3f look_at;
  int samples_a, samples_b;
  cv::Point3f color_a, color_b;

  // mesh transforms to apply
  ITransform* mesh_transform;
  ITransform* bg_transform;

protected:

  // 3D rendering details
  static const cv::Point3f up_direction;
  static const double fov;
  static const double z_clip_near;
  static const double z_clip_far;

  // Light properties
  static const GLfloat light_ambient[];
  static const GLfloat light_diffuse[];
  static const GLfloat light_specular[];
  static const GLfloat light_position[];

  // Default material properties
  static const GLfloat mat_ambient[];
  static const GLfloat mat_diffuse[];
  static const GLfloat mat_specular[];
  static const GLfloat high_shininess[];

  // Mouse projection stuff
  GLdouble proj_matrix[16], modelview_matrix[16];
  GLint viewport[4];
  bool mouse_enabled;
  QVector2D last_mouse_pos;

  // Keyboard and mouse event handlers
  void keyPressEvent  ( QKeyEvent   *e );
  void keyReleaseEvent( QKeyEvent   *e );
  void mouseMoveEvent ( QMouseEvent *e );
  void mousePressEvent( QMouseEvent *e );
  void leaveEvent     ( QEvent      *e );

  bool translateMouse(QVector2D pos, cv::Point3f& result);
};

#endif // DRUM_VIEWER_H
