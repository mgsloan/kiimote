#include "kw_controller.h"

KinectWiiController::KinectWiiController(KinectController* _kc, WiiController* _wc, ChronosController* _cc) :
  kc(_kc), wc(_wc), cc(_cc), stickZeroIsGreen(false) {
  if (kc) { kc->setListener(this); }
  if (wc) { wc->setListener(this); }
  if (cc) { cc->setListener(this); }
}

void KinectWiiController::addListener(IDrumstickListener* _listener) {
  listeners.push_back(_listener);
}

int KinectWiiController::countSticks() {
  return 4;//wc ? wc->countWiimotes() : 2;
}

bool KinectWiiController::getOrientation(int stick, cv::Point3f& result) {
  if (!wc || stick < 0) return false;
  int cnt = wc->countWiimotes();
  if (cnt > 0) {
      vector<cv::Point3f>* hist = wc->getHistory()[stick];
      if (hist != NULL && hist->size() > 0) {
          result = hist->back();
          return true;
      }
  }
  return false;
}

bool KinectWiiController::getPosition(int stick, cv::Point3f& result) {
  if (!kc) return false;

  // TODO: might want to generalize this to track more points.
  switch (stickZeroIsGreen ? stick : 1-stick) {
    case 0: result = kc->redPos(); return true;
    case 1: result = kc->greenPos(); return true;
    default: return false;
  }
}

bool KinectWiiController::getColor(int stick, cv::Point3f& result) {
  switch (stickZeroIsGreen ? stick : 1-stick) {
    case 0: result.x = 1; result.y = 0; result.z = 0; return true; 
    case 1: result.x = 0; result.y = 1; result.z = 0; return true;
    default: return false;
  }
}

void KinectWiiController::vibrateStick(int stick) {
  // TODO: support amplitude!
  if (wc) { wc->vibrateWiimote(stick); }
}

void KinectWiiController::swapSticks() {
  printf("stickZeroIsGreen = %i\n", stickZeroIsGreen);
  stickZeroIsGreen = !stickZeroIsGreen;
}

// IKinectListener
void KinectWiiController::handleKinectUpdate() {
  for(int li = 0; li < listeners.size(); li++) {
    IDrumstickListener* listener = listeners[li];
    // TODO: needs to report only when the sticks actually move,
    // and not report on untracked sticks.
    cv::Point3f ro, go, rp(kc->redPos()), gp(kc->greenPos());
    getOrientation(0, ro); getOrientation(1, go);
    listener->onStickMove( 0, stickZeroIsGreen ? rp : gp, ro);
    listener->onStickMove( 1, stickZeroIsGreen ? gp : rp, go);
    listener->onHeadMove( kc->headPos() );
  }
}

// IWiiListener
void KinectWiiController::wiiHit(int wmnum, float mag) {
  for(int li = 0; li < listeners.size(); li++) {
    IDrumstickListener* listener = listeners[li];
    listener->onStickHit(wmnum, mag);
  }
}

// IChronosListener
void KinectWiiController::chronosHit(float mag) {
  for(int li = 0; li < listeners.size(); li++) {
    IDrumstickListener* listener = listeners[li];
    listener->onStickHit(10, mag);
  }
}

void KinectWiiController::wiiButtonPress(int wmnum, int button, bool release) {
  for(int li = 0; li < listeners.size(); li++) {
    IDrumstickListener* listener = listeners[li];
    listener->onButtonPress(wmnum, button, release);
  }
}
