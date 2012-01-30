#ifndef VIEWER_H
#define VIEWER_H

#include <opencv2/core/core.hpp>
#include <QApplication>
#include <QGLWidget>

// Forward declaration of App for pointer.
class App;

class Viewer : public QGLWidget {
  Q_OBJECT

public:
//protected:
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

  App* app;

  cv::Point3f eye_position;
  cv::Point3f look_at;
  int samples_a, samples_b;
  cv::Point3f color_a, color_b;

  GLdouble proj_matrix[16], modelview_matrix[16];
  GLint viewport[4];
  QSize size;

//public:
  void paintGL();

  Viewer(App* a);
};

#endif // VIEWER_H