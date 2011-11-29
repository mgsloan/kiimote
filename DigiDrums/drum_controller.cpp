#include "drum_controller.h"
#include "kw_controller.h"

#include "wiiuse.h"
#include "utils.h"

DrumSetController::DrumSetController(DrumSet* _drum_set,
    ISoundPlayer* _player, DrumViewer* _viewer, IDrumsticks* _sticks) :
  drum_set(_drum_set), player(_player), viewer(_viewer), sticks(_sticks), 
    recorder(NULL) {
  drum_set->initSticks(sticks->countSticks());
  sticks->addListener(this);
//  viewer->mesh_transform = new Inflate(cv::Point3f(0,0,0),
//    cv::Point3f(1,1,1), 0.3);
  viewer->mesh_transform = new ColorSpheres(5.0, 0.005);
  viewer->bg_transform = new Undulate2(cv::Point3f(0, 0, -3) , cv::Point3f(0, 0, -20), 100);
}

void DrumSetController::onStickMove(int stick, cv::Point3f position, cv::Point3f orientation) {
  // Determine the current drum to recolor it.
  // TODO: should this happen inside DrumSet?
  DrumSet::DrumStick& s = drum_set->sticks[stick];
  s.visible = true;
  s.pos = position;
  s.orient = orientation;
  sticks->getColor(stick, s.color);
  s.drum = drum_set->intersects(s.getTip());
//  printf("\nStick moved to drum %d at (%f, %f, %f)", s.drum,
//    position.x, position.y, position.z);
//  fflush(stdout);
}

void DrumSetController::onHeadMove(cv::Point3f position) {
  //viewer->look_at = position;
  //viewer->look_at.y = 0;
//  Inflate* inflate = (Inflate*) viewer->mesh_transform;
//  inflate->origin = position;
  Undulate2* undulate = (Undulate2*) viewer->bg_transform;
  undulate->history.erase(undulate->history.end() - 1);
  cv::Point3f old = undulate->history[0];
  old *= 0.5;
  undulate->history.insert(undulate->history.begin(), old);
  //recorder->recordFrame();
}

void DrumSetController::onStickLeave(int stick) {
  drum_set->sticks[stick].visible = false;
}

void DrumSetController::performHit(cv::Point3f pos, float loudness) {
  int drum_ix = drum_set->intersects(pos);
  player->playSound(drum_set->instrument, drum_ix, loudness);
}

void DrumSetController::onStickHit(int stick, float loudness) {
  if (stick == 10) {
    player->playSound(drum_set->instrument, 4, loudness);
  }
  // Tell the sound player to play a sound.
  int drumix = drum_set->sticks[stick].drum;
  printf("hit %i\n", drum_set->sticks[stick].drum);
  if (drumix >= 0) {
    player->playSound(drum_set->instrument, drum_set->drums[drumix]->sound, loudness);
    sticks->vibrateStick(stick);
    ColorSpheres* spheres = (ColorSpheres*) viewer->mesh_transform;
    spheres->add(drum_set->sticks[stick].pos * 0.1,
      drum_set->sticks[stick].color, loudness);
    Undulate2* undulate = (Undulate2*) viewer->bg_transform;
    undulate->history[0] += cv::Point3f(0, 0.4, 0);
  }
}

void DrumSetController::onButtonPress(int stick, int button, bool release) {
  // If the A button was pressed, create a drum there.
  cv::Point3f pos;
  if (button == WIIMOTE_BUTTON_A && !release && sticks->getPosition(stick, pos)) {
    // Create a new drum.
    printf("\nCreated new drum.");
    fflush(stdout);
    cv::Point3f orient;
    sticks->getOrientation(stick, orient);
    drum_set->addDrum(new Drum(pos, orient, 0.6, (drum_set->drums.size() + 1)
      % player->countSounds(drum_set->instrument)));
  } else if ((button == WIIMOTE_BUTTON_PLUS || button == WIIMOTE_BUTTON_MINUS)
      && !release && drum_set->sticks[stick].drum >= 0) {
    // Change the current drum sound.
    // TODO: provide a class method for this
    Drum* drum = drum_set->drums[drum_set->sticks[stick].drum];
    if (button == WIIMOTE_BUTTON_PLUS) {
      drum->sound = (drum->sound + 1) % player->countSounds(drum_set->instrument);
    } else {
      drum->sound = (drum->sound == 0) ? player->countSounds(drum_set->instrument) - 1 :
        drum->sound - 1;
    }
    onStickHit(stick, 0.7);
    printf("\nChanged sound of drum %d to %d", drum_set->sticks[stick].drum,
      drum->sound);
    fflush(stdout);
  } else if (button == WIIMOTE_BUTTON_DOWN) {
    int ix = drum_set->sticks[stick].drum;
    if (ix != -1 && drum_set->drums.size() > ix) {
        drum_set->drums.erase(drum_set->drums.begin() + ix);
        printf("\nRemoved drum. %i", ix);
    }
  } else if (button == WIIMOTE_BUTTON_UP) {

    cv::Point3f o; 
    if (sticks->getOrientation(stick, o)) {
        //printf("Got orientation! %f %f %f\n", o.x, o.y, o.z);
        viewer->eye_position += cv::Point3f(-o.x, 0, -o.z);
        viewer->eye_position.x *= 0.96;
        viewer->eye_position.z *= 0.96;
        viewer->look_at = cv::Point3f(0,0,-30);
    }

//      double roll = atan2( sqrt(Y*Y + X*X), Z) * 180/M_PI,
//             pitch = atan2( sqrt(X*X + Z*Z), Y) * 180/M_PI;

  } else if (button == WIIMOTE_BUTTON_RIGHT) {
    cv::Point3f o; 
    if (sticks->getOrientation(stick, o)) {
        viewer->eye_position += cv::Point3f(0, -o.z, 0);
        viewer->eye_position.y *= 0.9;
        viewer->look_at = cv::Point3f(0,0,-30);
    }
  } else if (button == WIIMOTE_BUTTON_LEFT) {
    KinectWiiController* kw = (KinectWiiController*) sticks;
    if (!release) kw->swapSticks();
    /*
  } else if (button == WIIMOTE_BUTTON_MINUS) {
    cv::Point3f o; 
    if(sticks->getOrientation(stick, o)) {
      // heh major code smell
      o *= -1.0 / mag(o);
      Undulate* undulate = (Undulate*) viewer->mesh_transform;
      undulate->travel = cv::Point3f(o.x * 0.1, o.y * 0.1, o.z * 0.1);
      undulate->flow   = cv::Point3f(o.y, -o.x, 0);
    }
  */
  } else if (button == WIIMOTE_BUTTON_ONE) {
        drum_set->instrument = 0;
  } else if (button == WIIMOTE_BUTTON_TWO) {
        drum_set->instrument = 1;
  } else if (button == WIIMOTE_BUTTON_B) {
    int drumix = drum_set->sticks[stick].drum;
    int& prev = drum_set->sticks[stick].prev_drum;
    if (drumix >= 0 && prev != drumix) {
      onStickHit(stick, 0.8);
      prev = drumix;
      player->playSound(drum_set->instrument, drum_set->drums[drumix]->sound, 1.0);
      sticks->vibrateStick(stick);
    }
  } /* else if (button == WIIMOTE_BUTTON_HOME) {
    if (!recorder) {
      float dur = -1;
      if (clones.size() > 0) {
        dur = clones[0]->recorder->times.back();
      }
      recorder = new DrumsticksRecorder((KinectWiiController*)sticks, this, 1.0f);
    } else {
      recorder->duration = 0;
    }
    recorder->recordFrame();
  }
  */
}
