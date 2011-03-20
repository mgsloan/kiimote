#ifndef KW_CONTROLLER_H
#define KW_CONTROLLER_H

#include "kinect_controller.h"
#include "wii_controller.h"
#include "chronos_controller.h"
#include "drum_viewer.h" // TODO: include just IDrumsticks

class KinectWiiController : public IDrumsticks, IKinectListener, IWiiListener, IChronosListener {
public:
  KinectController* kc;
  WiiController* wc;
  ChronosController* cc;

  bool stickZeroIsGreen;
public:
  vector<IDrumstickListener*> listeners;
  /** Creates a KinectWiiController using the Kinect and Wii drivers. */
  KinectWiiController(KinectController* _kc, WiiController* _wc, ChronosController* _cc);

  // IDrumsticks
  void addListener(IDrumstickListener* listener);
  int countSticks();
  bool getPosition(int stick, cv::Point3f& result);
  bool getOrientation(int stick, cv::Point3f& result);
  bool getColor(int stick, cv::Point3f& result);
  void vibrateStick(int stick);

  // Swap the two Wiimotes
  void swapSticks();

  // IKinectListener
  void handleKinectUpdate();

  // IChronosListener
  void chronosHit(float mag);

  // IWiiListener
  void wiiHit(int wmnum, float mag);
  void wiiButtonPress(int wmnum, int button, bool release);
};

#endif // KW_CONTROLLER_H
