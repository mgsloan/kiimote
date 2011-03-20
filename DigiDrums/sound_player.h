#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <vector>
#include <string>

/**
 * Interface for an object that plays drum sounds for the basic drum kit.
 *
 * This interface can be implemented by a class that plays sounds through a
 * library, or by a class that sends OSC signals to a ChucK client.
 */
class ISoundPlayer {
public:
  /**
   * Plays the sound of the given number with the given loudness,
   * where loudness varies from 0 (quiet) to 2 (loud).
   */
  virtual void playSound(int instrument, int sound, float loudness) = 0;
  
  /** Get the number of instruments */
  virtual unsigned int countInstruments() const = 0;
  
  /** Get number of sounds. */
  virtual unsigned int countSounds(int instrument) const = 0;

  /** Get number of loudness levels. */
  virtual unsigned int countLevels() const = 0;
  
};

/**
 * Generates the names of sound files with the indicated loudness.
 */
class SoundFileFinder {
protected:
  const char* base_path;
  std::vector<std::vector<char*> > sounds;
  const unsigned int* sound_sizes;
  const char** instruments;
  const unsigned int instrument_size;
  const char** levels;
  const unsigned int levels_size;
  const char* ext;

public:
  SoundFileFinder(const char* _base_path, const char* _instruments[],
  std::vector<std::vector<char*> > _sounds, const unsigned int _sound_sizes[], const char* _levels[], const unsigned int _levels_size, const unsigned int _instrument_size, const char* _ext);

  const std::string getFileName(const unsigned int instrument, const unsigned int sound,
    const unsigned int loudness) const;
  unsigned int countSounds(int instrument) const;
  unsigned int countLevels() const;
  unsigned int countInstruments() const;
};

#ifdef APLAY_SOUND_PLAYER
#include <QThread>

/**
 * Uses the aplay program to play sounds.
 */
class AplaySoundPlayer : public ISoundPlayer {
protected:
  const SoundFileFinder* sff;

  /**
   * One of these is launched for each sound that's played.
   * Really efficient, huh?
   */
  class SoundThread : public QThread {
  protected:
    std::string sound_path;
    void run();
  public:
    SoundThread(std::string _sound_path);
    virtual ~SoundThread();
  };

public:
  /**
   * Creates an AplaySoundPlayer with the given sounds.
   * The index of the sound in the vector is the sound number provided by
   * playSound.
   */
  AplaySoundPlayer(const SoundFileFinder *_sff);

  // ISoundPlayer
  void playSound(const int instrument, const int sound, const int loudness);
  unsigned int countSounds(int instrument) const;
  unsigned int countLevels() const;
  unsigned int countInstruments() const;
};

#endif // APLAY_SOUND_PLAYER

#ifdef OSC_SOUND_PLAYER
#include <lo/lo.h>

/**
 * Sends OSC signals to an application to play sounds.
 */
class OscSoundPlayer : public ISoundPlayer {
  lo_address addr;
  const char* path;

public:
  /**
   * Create a new OscSoundPlayer that sends OSC messages to the given host
   * and port number.
   */
  OscSoundPlayer(char* _host = NULL, char* _port = "6666", char* _path = "/drum/play");

  void playSound(int instrument, int sound, float loudness);
  unsigned int countSounds(int instrument) const;
  unsigned int countLevels() const;
  unsigned int countInstruments() const;
};

#endif // OSC_SOUND_PLAYER

#ifdef OPENAL_SOUND_PLAYER
#include <list>
#include <AL/al.h>
#include <AL/alut.h>

/**
 * Uses OpenAL to play sounds, performing manual mixing when
 * there are multiple streams.
 *
 * TODO: do 3D sound spatialization?
 */
class OpenALSoundPlayer : public ISoundPlayer {
protected:
  const SoundFileFinder* sff;

  // List of sounds available to play.
  std::vector<std::vector<ALuint> > sounds;

  // Queue of sound sources.  The newest sources are at the
  // head of this list, so call pop_back() to get the source to play.
  std::list<ALuint> sources;
public:
  /**
   * Create a new OpenALSoundPlayer with the given list of .WAV sounds,
   * and the given maximum number of concurrently-playing sounds.
   */
  OpenALSoundPlayer(const SoundFileFinder* _sff, const int max_sources = 10);

  void playSound(int instrument, int sound, float loudness);
  unsigned int countSounds(int instrument) const;
  unsigned int countLevels() const;
  unsigned int countInstruments() const;
  unsigned int bucketLoudness(float loudness);
};

#endif // OPENAL_SOUND_PLAYER

#endif // SOUND_PLAYER_H
