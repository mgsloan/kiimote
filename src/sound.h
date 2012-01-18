#ifndef SOUND_H
#define SOUND_H

#include <lo/lo.h>

/**
 * Sends OSC signals to an application to play sounds.
 */
class OscSoundPlayer {
  lo_address addr;
  const char* path;

public:
  /**
   * Create a new OscSoundPlayer that sends OSC messages to the given host
   * and port number.
   */
  OscSoundPlayer(char* _host = NULL, char* _port = "6666", char* _path = "/drum/play");

  void playSound(int instrument, int sound, float loudness);
};

#endif // SOUND_H