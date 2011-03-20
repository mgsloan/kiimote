#ifndef IDRUMSTICKS_H
#define IDRUMSTICKS_H

#include <opencv2/core/core.hpp>

/**
 * Interface for drumstick listeners.
 */
class IDrumstickListener {
public:
  /**
   * Called when a stick moves, providing the stick's new position.
   */
  virtual void onStickMove(int stick, cv::Point3f position, cv::Point3f orientation);

  /**
   * Called when the user's head moves.
   */
  virtual void onHeadMove(cv::Point3f position);

  /**
   * Called when a stick is moved in a drumming motion.
   */
  virtual void onStickHit(int stick, float loudness);

  /**
   * Called when a button is pressed on one of the sticks.
   */
  virtual void onButtonPress(int stick, int button, bool release);

  /**
   * Called when a stick "vanishes" -- when the tracking system
   * is no longer able to track a stick.
   */
  virtual void onStickVanish(int stick);
};

/**
 * Interface for a class that provides data from the Wiimotes.
 */
class IDrumsticks {
public:
  // NOTE: You will need to make sure that classes implementing this interface
  // use the same number of loudness levels.

  /** Adds a class that will receive events. */
  virtual void addListener(IDrumstickListener* listener) = 0;

  /** Returns the number of connected drumsticks. */
  virtual int countSticks() = 0;

  /**
   * Returns true if a stick is visible, storing the position in result.
   */
  virtual bool getPosition(int stick, cv::Point3f& result) = 0;

  /**
   * Returns true if a stick is visible, storing the orientation in result.
   */
  virtual bool getOrientation(int stick, cv::Point3f& result) = 0;

  /**
   * Returns true if a stick is visible, storing the color in result.
   */
  virtual bool getColor(int stick, cv::Point3f& result) = 0;

  /**
   * Vibrates the given stick.
   *
   */
  virtual void vibrateStick(int stick) = 0;
};

#endif // IDRUMSTICKS_H
