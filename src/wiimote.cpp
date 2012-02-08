#include "wiimote.h"

#include "utils.h"

cv::Point3f WiiMote::getAccel() {
  return cv::Point3_<float>(wm->gforce.x, wm->gforce.y, wm->gforce.z);
}

bool WiiMote::button(unsigned ix) {
  return ix < 32 && (wm->btns & (1 << ix)) ? 1 : 0;
}

WiiManager::WiiManager() {
  wii_mote_ts = wiiuse_init(MAX_wii_motes);
}

WiiManager::~WiiManager() {
  wiiuse_cleanup(wii_mote_ts, MAX_wii_motes);
}

unsigned WiiManager::search() {
  //This will return the number of actual wii_motes that are in discovery mode.
  unsigned wii_found = wiiuse_find(wii_mote_ts, MAX_wii_motes, 5); // 5 = time delay.
  if (!wii_found) {
    printf ("No wii_motes found.");
    return 0;
  }

  //This will return the number of established connections to the found wii_motes.
  unsigned wii_count = wiiuse_connect(wii_mote_ts, MAX_wii_motes);
  if (wii_count)
    printf("Connected to %i wii_motes (of %i found).\n", wii_count, wii_found);
  else {
    printf("Failed to connect to any wiimote.\n");
    return 0;
  }

  wii_motes.clear();
  for (unsigned i = 0; i < wii_count; ++i) {
    wii_motes.push_back(WiiMote(wii_mote_ts[i]));
  }

  /*
   *    Now set the LEDs and rumble for a second so it's easy
   *    to tell which wii_motes are connected (just like the wii does).
   */
  for (unsigned i = 0; i < std::min(4u, wii_count); i++)
    wiiuse_set_leds(wii_mote_ts[i], WIIMOTE_LED_1 * (1 << i));

  for (unsigned i = 0; i < wii_count; i++) {
    wiiuse_rumble(wii_mote_ts[i], 1);
    wiiuse_motion_sensing( wii_mote_ts[i], 1);
  }

  sleep_ms(200);

  for (unsigned i = 0; i < wii_count; i++)
    wiiuse_rumble(wii_mote_ts[i], 0);
  return wii_count;
}

bool WiiManager::poll() {
  if (wii_motes.size() == 0) return false;
  if (wiiuse_poll(wii_mote_ts, wii_motes.size())) {
    for (unsigned i = 0; i < wii_motes.size(); ++i) {
      if (wii_mote_ts[i]->event == WIIUSE_UNEXPECTED_DISCONNECT ||
          wii_mote_ts[i]->event == WIIUSE_DISCONNECT)
        return false;
    }
  }
  return true;
}