#ifndef KINECT_H
#define KINECT_H

class Kinect : public ntk::AsyncEventListener {
 public:
  ntk::KinectGrabber grabber;
  ntk::KinectProcessor processor;
  ntk::RGBDCalibration calib_data;

  ntk::RGBDImage last_image;
  ntk::Mesh generated_mesh;

  Kinect(const char* calibration_file);

  // TrackedPoint& getTracker(const ColorProfile& profile)

  // Update the tracking data.
  void update();

  // Handle a grabber event.
  void handleAsyncEvent();
}

#endif
