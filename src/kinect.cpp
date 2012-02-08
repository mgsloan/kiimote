#include "kinect.h"

#include "tracker.h"

#include <ntk/mesh/rgbd_modeler.h>

Kinect::Kinect(const char* calibration_file)
  : red(20, PointMetric(), 1, 4)
  , green(20, PointMetric(), 1, 4)
  , head(80, PointMetric(), -1, 1)
  , has_data(false)
  , do_near_clip(true)
  , do_tracking(true)
  , do_calibration(true)
  {
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

void Kinect::handleAsyncEvent(ntk::EventListener::Event e) {
  grabber.copyImageTo(last_image);
  processor.processImage(last_image);

  if (bg_mesh.vertices.size() == 0) {
    generator->generate(last_image);
    bg_mesh = generator->mesh();
  }

  // Clip to near range.
  if (do_near_clip) {
    clipNear(last_image);
  }

  generator->generate(last_image);
  generated_mesh = generator->mesh();
  has_data = true;

  if (do_tracking) {
    cv::Point3f fudge(0,0,-0.1);

    cv::Point3f hp, rp, gp;
    rp = red.computeValue(); gp = green.computeValue();
    rp *= 0.1;
    gp *= 0.1;
    trackHands(last_image, rp, gp);
    trackHead(last_image, hp);
//    printf ("track red %f %f %f\n", rp.x, rp.y, rp.z);
//    printf ("track green %f %f %f\n", gp.x, gp.y, gp.z);
    if (fabs(rp.z) > 0.1 && rp.x > -1.3) red.push(rp * 10 + fudge);
    if (fabs(gp.z) > 0.1 && gp.x > -1.3) green.push(gp * 10 + fudge);
    head.push(hp * 10);
  }

  if (do_calibration) {
    calib_gpos = cv::Point3f(3,0,-7);
    red_profile.colors.clear();
    green_profile.colors.clear();
    for (int i = 0; i < generated_mesh.vertices.size(); i++) {
      cv::Point3f delta = generated_mesh.vertices[i] * 10 - calib_rpos;
      delta.z = 0;
      double mag = sqrt(delta.dot(delta));
      if (mag < 0.3) {
        red_profile.addColor(generated_mesh.colors[i]);
        continue;
      }
      delta = generated_mesh.vertices[i] * 10 - calib_gpos;
      mag = sqrt(delta.dot(delta));
      if (mag < 0.3) {
        green_profile.addColor(generated_mesh.colors[i]);
      }
    }
    red_profile.calculateStatistics();
    green_profile.calculateStatistics();
  }

  grabber.newEvent();
}