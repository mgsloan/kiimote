#include "utils.h"

void setMeshColor(ntk::Mesh& mesh, cv::Vec3b color) {
  for (int i = 0; i < mesh.vertices.size(); i++) {
    mesh.colors.push_back(color);
  }
}

void transformMesh(ntk::Mesh& mesh, 
  float a11, float a12, float a13, float a14,
  float a21, float a22, float a23, float a24,
  float a31, float a32, float a33, float a34) {

  float elems[] = {a11, a12, a13, a14,
                   a21, a22, a23, a24,
                   a31, a32, a33, a34,
                   0, 0, 0, 1};

  cv::Mat transform(4, 4, CV_32F, &elems);
  ntk::Pose3D pose;
  pose.setCameraTransform(transform);
  mesh.applyTransform(pose);
}

void addSegment(ntk::Mesh& mesh, cv::Point3f fr, cv::Point3f to, float width) {
  mesh.vertices.push_back(fr);
  mesh.vertices.push_back(to);
  mesh.vertices.push_back(to - cv::Point3f(width,0,0));
  int ix = mesh.vertices.size();
  mesh.faces.push_back(ntk::Face(ix - 3, ix - 2, ix -1));
  mesh.colors.push_back(cv::Vec3b(255,255,255));
}

void sleep_ms(int ms) {
    #ifndef WIN32
        usleep(ms * 1000);
    #else
        Sleep(ms);
    #endif
}
timeval tv;
long startsec;
bool time_started = false;

void start_timer() {
  if (!time_started) {
    gettimeofday(&tv, NULL);
    startsec = tv.tv_sec;
  }
}

float get_time() {
  gettimeofday(&tv, NULL);
  int elapsed = tv.tv_sec - startsec;
  return (float)tv.tv_usec * 0.000001 + (float)elapsed;
}


