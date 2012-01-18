#ifndef SEEN_UTILS_H
#define SEEN_UTILS_H

#include <opencv2/core/core.hpp>

#include <sys/time.h>
#include <ntk/mesh/mesh.h>

void setMeshColor(ntk::Mesh& mesh, cv::Vec3b color);

void transformMesh(ntk::Mesh& mesh, 
  float a11, float a12, float a13, float a14,
  float a21, float a22, float a23, float a24,
  float a31, float a32, float a33, float a34);

void addSegment(ntk::Mesh& mesh, cv::Point3f fr, cv::Point3f to, float width);

void sleep_ms(int ms);

inline double mag(cv::Point3f pnt) { return sqrt(pnt.dot(pnt)); }

// shared timer
void start_timer();
float get_time();

#endif
