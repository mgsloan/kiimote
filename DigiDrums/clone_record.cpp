#include "clone_record.h"
#include "drum_controller.h"
#include "kw_controller.h"
#include "utils.h"
#include <algorithm>

#define PSZ 256

DrumsticksRecorder::DrumsticksRecorder(KinectWiiController* kwc, DrumSetController* ds, float dur) :
  input(kwc), drum_controller(ds), duration(dur), times(PSZ), meshes(PSZ),
  hit_time(PSZ), hit_amp(PSZ), hit_pos(PSZ) {
  start_timer();
  start_time = get_time();
  kwc->addListener(this);
}

void DrumsticksRecorder::onStickHit(int stick, float amp) {
  hit_time.push_back(get_time() - start_time);
  hit_amp.push_back(amp);
  hit_pos.push_back(drum_controller->drum_set->sticks[stick].getTip());
}

void DrumsticksRecorder::recordFrame() {
  float time = get_time();
  if (duration < 0 || time - start_time > duration) {
    std::remove(input->listeners.begin(), input->listeners.end(), this);
    drum_controller->clones.push_back(playback());
    drum_controller->recorder = NULL;
  }
  times.push_back(time);
  meshes.push_back(input->kc->getMesh());
}

PlaybackThread::PlaybackThread(DrumsticksRecorder* r, float st, DrumSetController* ds) :
recorder(r), start_time(st), cur_ix(0), drum_controller(ds) {}

void PlaybackThread::run() {
  float length = recorder->times.back();
  int hit_ix = 0;
  while(true) {
    float time = get_time() - start_time;
    time -= length * floor(time / length);
    for (; cur_ix < recorder->meshes.size(); cur_ix++) {
      if (recorder->times[cur_ix] > time) break;
    }
    for (; hit_ix < recorder->hit_time.size(); hit_ix++) {
      if (recorder->hit_time[hit_ix] > time) break;
      drum_controller->performHit(recorder->hit_pos[hit_ix], 
                                  recorder->hit_amp[hit_ix]);
    }
  }
}

ntk::Mesh& PlaybackThread::getMesh() {
  return recorder->meshes[cur_ix];
}

PlaybackThread* DrumsticksRecorder::playback() {
  PlaybackThread* pt = new PlaybackThread(this, get_time(), drum_controller);
  pt->start();
  return pt;
}

/*
void DrumsticksRecorder::addListener(IDrumstickListener* listener) {
  drum_listeners.push_back(listener);
}
bool DrumsticksRecorder::getPosition(int stick, cv::Point3f& result) {
  if (mode != 2) return;
  result = positions[getIndex()];
  return true;
}

bool DrumsticksRecorder::getOrientation(int stick, cv::Point3f& result) {
  if (mode != 2) return;
  result = orientations[getIndex()];
  return true;
}

bool DrumsticksRecorder::getColor(int stick, cv::Point3f& result) {
  if (mode != 2) return;
  if (stick == 0) result = cv::Point3f(1, 0, 0);
  if (stick == 1) result = cv::Point3f(0, 1, 0);
  return true;
}

void DrumsticksRecorder::vibrateStick(int stick) {
  if (mode != 2) return;
}
*/


