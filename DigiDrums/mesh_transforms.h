#ifndef KINECT_TRANSFORMS_H
#define KINECT_TRANSFORMS_H


#include <sys/time.h>
#include <opencv2/core/core.hpp>

#include <ntk/mesh/mesh.h>

void mergeMesh(const ntk::Mesh& a, ntk::Mesh&b);

class ITransform {
 public:
  virtual void apply(ntk::Mesh& mesh) = 0;
};

class Undulate : public ITransform {
 public:
  cv::Point3f travel, flow;

  Undulate(cv::Point3f t, cv::Point3f f);

  virtual void apply(ntk::Mesh& mesh);
};

class Undulate2 : public ITransform {
 public:
  cv::Point3f origin, travel;
  vector<cv::Point3f> history;
  
  Undulate2(cv::Point3f o, cv::Point3f t, int sz);

  virtual void apply(ntk::Mesh& mesh);
};

class Inflate : public ITransform {
 public:
  cv::Point3f origin, bias;
  long startsec;
  double thresh;

  Inflate(cv::Point3f o, cv::Point3f b, double t);

  virtual void apply(ntk::Mesh& mesh);
};

class ColorSpheres : public ITransform {
 public:
  vector<cv::Point3f> origins;
  vector<cv::Point3f> colors;
  vector<float> alphas;
  vector<float> start_times;
  float lifetime, width;

  ColorSpheres(float life, float w);

  void add(cv::Point3f origin, cv::Point3f color, float alpha);

  virtual void apply(ntk::Mesh& mesh);
};

class TransformGroup : public ITransform {
 public:
  vector<ITransform*> transforms;

  TransformGroup();

  void push(ITransform* t);

  virtual void apply(ntk::Mesh& mesh);
};

#endif
