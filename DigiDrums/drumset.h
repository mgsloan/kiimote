#ifndef DRUMSET_H
#define DRUMSET_H

#include <opencv2/core/core.hpp>
#include <vector>
#include "sound_player.h"
#include "idrumsticks.h"

/**
 * A surface for the tip of the player's drumstick to hit.
 *
 * TODO: track pitch and roll so the side of the drumstick can hit the
 * drum as well!
 *
 * TODO: put inside class DrumSet?
 */
class Drum {
public:
  cv::Point3f location;
  cv::Point3f orientation;
  double radius;
  int sound;

  Drum(cv::Point3f _location, cv::Point3f _orientation, double _radius, int _sound);

  virtual void draw(cv::Point3f color);
};

/**
 * A list of drums, which can determine which drum would be hit at a given
 * point, draw the individual drums, and track and draw the drumsticks.
 */
class DrumSet {
public:
   int instrument;
  /**
   * Tracks a drum stick's position and current drum.
   */
  class DrumStick {
  public:
    cv::Point3f pos;
    cv::Point3f orient;
    cv::Point3f color;
    bool visible;
    int drum;
    int prev_drum;

    DrumStick();
    DrumStick(int _drum);
    cv::Point3f getTip();
  };

  static const double radius_threshold = 5;

  std::vector<Drum*> drums;
  std::vector<DrumStick> sticks;

  DrumSet();

  void initSticks(int num_sticks);
  void addDrum(Drum* drum);
  void setInstrument(int _instrument);
  int intersects(const cv::Point3f point);

  // TODO: do drawing in a view class.
  void draw();

  ~DrumSet();
};

#endif // DRUMSET_H
