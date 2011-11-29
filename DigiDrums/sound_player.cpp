#include "sound_player.h"

#include "stdio.h"
#include "stdlib.h"

#include <lo/lo.h>

using namespace std;

SoundFileFinder::SoundFileFinder(const char* _base_path, const char* _instruments[],
   std::vector<std::vector<char*> > _sounds, const unsigned int _sound_sizes[], const char* _levels[], 
  const unsigned int _levels_size, const unsigned int _instrument_size, const char* _ext) :
  base_path(_base_path), sounds(_sounds), sound_sizes(_sound_sizes),
  levels(_levels), levels_size(_levels_size), instruments(_instruments), instrument_size(_instrument_size), ext(_ext) {}

const string SoundFileFinder::getFileName(const unsigned int instrument, const unsigned int sound,
  const unsigned int loudness) const {
  string filename = base_path;
  filename += instruments[instrument];
  filename += sounds[instrument][sound];
  filename += levels[loudness];
  filename += ext;
  return filename;
}

unsigned int SoundFileFinder::countSounds(int instrument) const { return sound_sizes[instrument]; }

unsigned int SoundFileFinder::countLevels() const { return levels_size; }

unsigned int SoundFileFinder::countInstruments() const { return instrument_size; }

#ifdef APLAY_SOUND_PLAYER
AplaySoundPlayer::SoundThread::SoundThread(std::string _sound_path) :
  sound_path(_sound_path) {
  /** Make this thread delete itself when finished. */
  QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

AplaySoundPlayer::SoundThread::~SoundThread() {
  printf("\nCompleted sound thread.");
}

void AplaySoundPlayer::SoundThread::run() {
  string sound = "/usr/bin/aplay ";
  sound += sound_path;
  printf("playing sound %s\n", sound_path.c_str());
  printf("invoking %s\n", sound.c_str());
  system(sound.c_str());
}

AplaySoundPlayer::AplaySoundPlayer(const SoundFileFinder *_sff) :
  sff(_sff) {}

void AplaySoundPlayer::playSound(const int instrument, const int sound, const float loudness) {
  (new SoundThread(sff->getFileName(sound, loudness)))->start();
}

unsigned int AplaySoundPlayer::countSounds() const {
  return sff->countSounds();
}

unsigned int AplaySoundPlayer::countLevels() const {
  return sff->countLevels();
}

unsigned int AplaySoundPlayer::countInstruments() const {
  return sff->countInstruments();
}


#endif // APLAY_SOUND_PLAYER

#ifdef OSC_SOUND_PLAYER
OscSoundPlayer::OscSoundPlayer(const char* _host,
    const char* _port, const char* _path) :
  addr(lo_address_new(_host, _port)), path(_path) {}

void OscSoundPlayer::playSound(const int instrument, const int sound, const float loudness) {
  lo_send(addr, path, "iif", instument, sound, loudness);
}
#endif // OSC_SOUND_PLAYER

#ifdef OPENAL_SOUND_PLAYER
OpenALSoundPlayer::OpenALSoundPlayer(const SoundFileFinder* _sff,
  const int max_sources) : sff(_sff) {

  // Set up listener parameters.
  const ALfloat listenerPos[]={0.0,0.0,0.0};
  const ALfloat listenerVel[]={0.0,0.0,0.0};
  const ALfloat listenerOri[]={0.0,0.0,1.0, 0.0,1.0,0.0};

  alListenerfv(AL_POSITION, listenerPos);
  alListenerfv(AL_VELOCITY, listenerVel);
  alListenerfv(AL_ORIENTATION, listenerOri);

  // Load the sounds.
  int totalSoundCount = 0;
  for(int m = 0; m < sff->countInstruments(); m++) {
    totalSoundCount += sff->countSounds(m);
    sounds.push_back(std::vector<ALuint>());
  }
  ALuint* buffers = new ALuint[totalSoundCount * sff->countLevels()];
  alGenBuffers(totalSoundCount * sff->countLevels(), buffers);
  int offset = 0;
  for(int j = 0; j < sff->countInstruments(); j++) {
      for (int i = 0; i < sff->countSounds(j); i++) {
        // Sound properties
        ALsizei size, freq;
        ALenum format;
        ALvoid *data;
        ALboolean loop;

        // Load .WAV file into buffer.
        // TODO: error handling!
        // TODO: put suffixes elsewhere
        for (int s = 0; s < sff->countLevels(); s++) {
          string file = sff->getFileName(j, i, s);
          alutLoadWAVFile((ALbyte*)(file.c_str()), &format, &data, &size,
            &freq, &loop);
          alBufferData(buffers[offset + i * sff->countLevels() + s], format, data, size, freq);
          alutUnloadWAV(format, data, size, freq);
          // TODO: can't we just use the buffer array?
          sounds[j].push_back(buffers[offset + i * sff->countLevels() + s]);
        }
      }
      offset += countSounds(j);
  }
  delete [] buffers;

  // Initialize the sources.
  ALuint* source_handles = new ALuint[max_sources];
  alGenSources(max_sources, source_handles);
  for (int i = 0; i < max_sources; i++) {
    int source = source_handles[i];

    const ALfloat pos[]={0.0,0.0,0.0};
    const ALfloat vel[]={0.0,0.0,0.0};

    alSourcef(source, AL_PITCH, 1.0f);
    alSourcefv(source, AL_POSITION, pos);
    alSourcefv(source, AL_VELOCITY, vel);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    sources.push_front(source);
  }
  delete [] source_handles;
}

void OpenALSoundPlayer::playSound(const int instrument, const int sound, const float loudness) {
  // Put sound source at front of queue.
  ALuint source = *(sources.rbegin());
  sources.pop_back();
  sources.push_front(source);
  int play_level = bucketLoudness(loudness);
  // Play the sound from this source.
  alSourceStop(source);
  alSourcei(source, AL_BUFFER, sounds[instrument][sound * sff->countLevels() + play_level]);
  alSourcef(source, AL_GAIN, play_level / (sff->countLevels() - 1.0f));
  alSourcePlay(source);
  //const char* _path = "/drum/play";
  //lo_send(addr(lo_address_new(10.0.7.27, 6666)), _path, "iif", instument, sound, loudness);

}

unsigned int OpenALSoundPlayer::countSounds(int instrument) const {
  return sff->countSounds(instrument);
}

unsigned int OpenALSoundPlayer::countLevels() const {
  return sff->countLevels();
}

unsigned int OpenALSoundPlayer::countInstruments() const {
  return sff->countInstruments();
}

unsigned int OpenALSoundPlayer::bucketLoudness(float loudness) {
    if(loudness < .25){
        return 0;
    } else if(loudness < .75){
        return 1;
    }else{
        return 2;
    }
}
#endif // OPENAL_SOUND_PLAYER
