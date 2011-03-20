#include "wiimote.h"

cv::Point3f Wiimote :: getAccel() { return cv::Point3_<float>(wm->gforce.x, wm->gforce.y, wm->gforce.z); }

bool Wiimote :: button(unsigned ix) {
  return ix >= 0 && ix < 32 && (wm->btns & (1 << ix)) ? 1 : 0;
}

unsigned WiiController::search() {
  //This will return the number of actual wii_motes that are in discovery mode.
  wii_found = wiiuse_find(wii_motes, MAX_wii_motes, 5); // 5 = time delay.
  if (!wii_found) {
    printf ("No wii_motes found.");
    return 0;
  }

  //This will return the number of established connections to the found wii_motes.
  wii_connected = wiiuse_connect(wii_motes, MAX_wii_motes);
  if (wii_connected)
    printf("Connected to %i wii_motes (of %i found).\n", wii_connected, wii_found);
  else {
    printf("Failed to connect to any wiimote.\n");
    return 0;
  }

  /*
   *    Now set the LEDs and rumble for a second so it's easy
   *    to tell which wii_motes are connected (just like the wii does).
   */
  for (unsigned i = 0; i < min(4, wii_connected); i++)
    wiiuse_set_leds(wii_motes[i], WIIMOTE_LED_1 * (1 << i));

  for (unsigned i = 0; i < wii_connected; i++) {
    wiiuse_rumble(wii_motes[i], 1);
    wiiuse_motion_sensing( wii_motes[i], 1);
  }

  sleep_ms(200);

  for (unsigned i = 0; i < wii_connected; i++)
    wiiuse_rumble(wii_motes[i], 0);
  return wii_connected;
}

void WiiThread::run() {
  wiimote** wii_motes = wiiuse_init(MAX_wii_motes);
  unsigned connected;
  while (1) {
    if (!connected) {
      connected = search();
      for (unsigned i = 0; i < connected; i++) {
        wiimotes.push_back(Wiimote(wii_motes[i]))
      }
    } else {
      // call update callback
    }
  }

  wiiuse_cleanup(wii_motes, MAX_wii_motes);
}
