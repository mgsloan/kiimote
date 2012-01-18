#ifndef KINECT_H
#define KINECT_H

#include <ntk/ntk.h>
#include <ntk/core.h>

#include <ntk/mesh/mesh.h>
#include <ntk/mesh/mesh_generator.h>

#include <ntk/camera/calibration.h>
#include <ntk/camera/rgbd_image.h>
#include <ntk/camera/rgbd_grabber.h>
#include <ntk/camera/kinect_grabber.h>

#include <ntk/thread/event.h>

class Kinect : public ntk::AsyncEventListener {
public:
//protected:
  ntk::KinectGrabber grabber;
  ntk::KinectProcessor processor;
  ntk::RGBDCalibration calib_data;
  ntk::MeshGenerator* generator;

  ntk::RGBDImage last_image;
  ntk::Mesh bg_mesh;
  ntk::Mesh generated_mesh;

  SmoothFilter<cv::Point3f, PointMetric>
    red, green, head;
  
  bool do_near_clip;
  bool do_tracking;

  bool do_calibration;
  ColorProfile red_profile, green_profile;
  cv::Point3f calib_rpos, calib_gpos;

//public:
  Kinect(const char* calibration_file);

  // Handle a grabber event.
  void handleAsyncEvent();
};

#endif // KINECT_H