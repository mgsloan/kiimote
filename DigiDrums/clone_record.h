#ifndef CLONE_RECORD_H
#define CLONE_RECORD_H

#include "idrumsticks.h"
#include <QThread>
#include <ntk/mesh/mesh.h>
  
class DrumSetController;
class KinectWiiController;
class DrumsticksRecorder;

class PlaybackThread : public QThread {
  Q_OBJECT

public:
  DrumsticksRecorder* recorder;
  int cur_ix;
  DrumSetController* drum_controller;
  float start_time;
  PlaybackThread(DrumsticksRecorder* rec, float st, DrumSetController* ds);
  virtual void run();
  ntk::Mesh& getMesh();
};

class DrumsticksRecorder : public IDrumstickListener {
 public:
  KinectWiiController* input;
  DrumSetController* drum_controller;

  vector<float> times;
  vector<ntk::Mesh> meshes;

  //TODO: use boost tuples.
  vector<float> hit_time, hit_amp;
  vector<cv::Point3f> hit_pos;

  float start_time, duration;
  
  DrumsticksRecorder(KinectWiiController* cont, DrumSetController *ds, float dur);

  void recordFrame();
  PlaybackThread* playback();

  // IDrumstickListener
  void onStickHit(int stick, float loudness);
};

#endif
