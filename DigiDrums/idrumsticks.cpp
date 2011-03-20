#include "idrumsticks.h"

// Stub methods for the event handlers in an IDrumstickListener.
void IDrumstickListener::onStickMove(int stick, cv::Point3f position, cv::Point3f orientation) {}
void IDrumstickListener::onStickHit(int stick, float loudness) {}
void IDrumstickListener::onHeadMove(cv::Point3f position) {}
void IDrumstickListener::onButtonPress(int stick, int button, bool release) {}
void IDrumstickListener::onStickVanish(int stick) {}
