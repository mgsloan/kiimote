#include "kinect_controller.h"
#include "tracker.h"

using namespace std;
using namespace ntk;

KinectController::KinectController(const char* calibration_file) :
    listener(NULL),
    red(20, PointMetric(), 1, 4),
    green(20, PointMetric(), 1, 4),
    head(80, PointMetric(), -1, 1),
    calibrating(true) {
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
  //printf("Began processing...");
  //fflush(stdout);
  // Grab the latest rgb / depth 
  grabber.copyImageTo(last_image);

  // Process it
  processor.processImage(last_image);

  if(bg_mesh.vertices.size() == 0) {
    generator->generate(last_image);
    bg_mesh = generator->mesh();
  }

  // Clip to near range.
  clipNear(last_image);

  //if (track_stuff) {
    cv::Point3f fudge(0,0,-0.1);

    cv::Point3f hp, rp, gp;
    rp = red.computeValue(); gp = green.computeValue();
    rp *= 0.1;
    gp *= 0.1;
    trackHands(last_image, rp, gp);
    trackHead(last_image, hp);
    printf ("track red %f %f %f\n", rp.x, rp.y, rp.z);
    printf ("track green %f %f %f\n", gp.x, gp.y, gp.z);
    if (fabs(rp.z) > 0.1 && rp.x > -1.3) red.push(rp * 10 + fudge);
    if (fabs(gp.z) > 0.1 && gp.x > -1.3) green.push(gp * 10 + fudge);
    head.push(hp * 10);

/*
    list<cv::Point3f>& rhist = red.getHistory();
    list<cv::Point3f>& ghist = green.getHistory();
    list<cv::Point3f>::iterator it;
    printf("[");
    for (it = rhist.begin(); it != rhist.end(); it++) {
        printf("%f, ", it->z);
    }
    printf("]\n%f\n", red.computeValue().z);
 */

    if (generator) {
      generator->generate(last_image);
      generated_mesh = generator->mesh();
      if (calibrating) {
        calibrating = false;


//        cv::Point3f hpos = head.computeValue();
//        calib_rpos = hpos + cv::Point3f(-1.5,-1.5,1.5);
//        calib_gpos = hpos + cv::Point3f(1.5,-1.5,1.5);

        calib_rpos = cv::Point3f(-3,0,-7);
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
    }

    // Alert data receiver with tracking data.
    //printf("Finished processing...");
    //fflush(stdout);
    if (listener) {
      listener->handleKinectUpdate();
    }

    // TODO: how do we prevent the event from firing if we need to halt the
    // data collection?
    grabber.newEvent();
  //}
}

ntk::Mesh& KinectController::getMesh() { return generated_mesh; }
ntk::Mesh& KinectController::getBgMesh() { return bg_mesh; }

void KinectController::setListener(IKinectListener* _listener) { listener = _listener; }

RGBDImage& KinectController::lastImage() { return last_image; }
cv::Point3f KinectController::headPos() { return head.computeValue(); }
cv::Point3f KinectController::redPos() { return red.computeValue(); }
cv::Point3f KinectController::greenPos() { return green.computeValue(); }
