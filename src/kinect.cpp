#include <ntk/ntk.h>
#include <ntk/core.h>

#include <ntk/mesh/rgbd_modeler.h>
#include <ntk/mesh/mesh_generator.h>
#include <ntk/camera/calibration.h>
#include <ntk/camera/rgbd_image.h>
#include <ntk/camera/rgbd_grabber.h>
#include <ntk/camera/kinect_grabber.h>

KinectController::KinectController(const char* calibration_file) {
  // Initialize the grabber.
  grabber.initialize();
  grabber.setIRMode(false);
  // TODO: add synchronous option?

  // Set calibration data.
  calib_data.loadFromFile(calibration_file);
  grabber.setCalibrationData(calib_data);

  generator = new ntk::MeshGenerator();
  generator->setMeshType(ntk::MeshGenerator::PointCloudMesh);
  generator->setUseColor(true);
  generator->setResolutionFactor(0.5);

  // Start grabbing data.
  grabber.addEventListener(this);
  grabber.start();
}

void KinectController::handleAsyncEvent() { update(); }

void KinectController::update() {
  grabber.copyImageTo(last_image);
  processor.processImage(last_image);

  if (generator) {
    generator->generate(last_image);
    generated_mesh = generator->mesh();
  }
}
