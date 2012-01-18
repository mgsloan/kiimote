#include "sound.h"

void OscSoundPlayer::playSound(const int instrument, const int sound, const float loudness) {
  lo_send(addr, path, "iif", instument, sound, loudness);
}