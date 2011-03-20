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
  ntk::KinectGrabber grabber;
  ntk::KinectProcessor processor;
  ntk::RGBDCalibration calib_data;
  ntk::MeshGenerator* generator;

  ntk::RGBDImage last_image;
  ntk::Mesh generated_mesh;

  Kinect(const char* calibration_file);

  // TrackedPoint& getTracker(const ColorProfile& profile)

  // Update the tracking data.
  void update();

  // Handle a grabber event.
  void handleAsyncEvent();
};

#endif
