#ifndef KINECT_CONTROLLER_H
#define KINECT_CONTROLLER_H

#include <ntk/ntk.h>
#include <ntk/core.h>

#include <ntk/mesh/rgbd_modeler.h>
#include <ntk/mesh/mesh_generator.h>
#include <ntk/mesh/surfels_rgbd_modeler.h>

#include <ntk/camera/calibration.h>
#include <ntk/camera/rgbd_image.h>
#include <ntk/camera/rgbd_grabber.h>
#include <ntk/camera/kinect_grabber.h>
#include <ntk/thread/event.h>

#include <opencv2/core/core.hpp>

#include <unistd.h>
#include <string>

#include "tracker.h"

#include "idrumsticks.h"
#include "smooth_filter.h"

class IKinectListener {
public:
  virtual void handleKinectUpdate() = 0;
};

/**
 * Obtains data from an attached Microsoft Kinect sensor, and performs
 * processing to yield the 3D positions of the two Wiimotes and the user's head.
 */
class KinectController : public ntk::AsyncEventListener {
protected:
  IKinectListener* listener;
  ntk::RGBDImage last_image;
  ntk::KinectGrabber grabber;
  ntk::KinectProcessor processor;
  ntk::Mesh generated_mesh;
  ntk::Mesh bg_mesh;
  //TODO: ensure this gets freed.
  ntk::MeshGenerator* generator;

  ntk::RGBDCalibration calib_data;

  SmoothFilter<cv::Point3f, PointMetric>
    red, green, head;

public:

  bool calibrating;
  ColorProfile red_profile, green_profile;
  cv::Point3f calib_rpos, calib_gpos;

  /** Create a new KinectController with the given calibration data. */
  KinectController(const char* calibration_file);

  /** Update the tracking data. */
  void update();

  /** Handle a grabber event. */
  void handleAsyncEvent();

  /** Obtain the last Kinect color and depth image. */
  ntk::RGBDImage& lastImage();

  /** Get the last generated mesh. Note that meshes are only generated when
    * generator is non-NULL.
    */
  ntk::Mesh& getMesh();
  ntk::Mesh& getBgMesh();

  /** Get the head position. */
  cv::Point3f headPos();

  /** Get the red point position. */
  cv::Point3f redPos();

  /** Get the green point position. */
  cv::Point3f greenPos();

  /** Set the listener. */
  void setListener(IKinectListener* _listener);
};

#endif // KINECT_CONTROLLER_H
