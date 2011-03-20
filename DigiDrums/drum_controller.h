#ifndef DRUM_CONTROLLER_H
#define DRUM_CONTROLLER_H

#include "clone_record.h"
#include "drumset.h"
#include "sound_player.h"
#include "idrumsticks.h"

class DrumViewer;

/**
 * Controls a drum set.
 */
class DrumSetController : public IDrumstickListener {
public:
  DrumSet* drum_set;
  ISoundPlayer* player;
  DrumViewer* viewer;
  IDrumsticks* sticks;

  DrumsticksRecorder* recorder;
  vector<PlaybackThread*> clones;

  DrumSetController(DrumSet* _drum_set, ISoundPlayer* _player,
    DrumViewer* _viewer, IDrumsticks* _sticks);

  void performHit(cv::Point3f pos, float loud);

  void onStickMove(int stick, cv::Point3f position, cv::Point3f orientation);
  void onHeadMove(cv::Point3f position);
  void onStickLeave(int stick);
  void onStickHit(int stick, float loudness);
  void onButtonPress(int stick, int button, bool release);
};

#endif // DRUM_CONTROLLER_H
