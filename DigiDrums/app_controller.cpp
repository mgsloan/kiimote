#include "app_controller.h"

using namespace std;

// Command-line options
namespace opt
{
  ntk::arg<bool> no_kinect("--no_kinect", "Do not depend on kinect input", 0);
}

AppController::AppController(int argc, char* argv[])
 : QApplication(argc, argv), kcp(NULL) {
  QApplication::setGraphicsSystem("raster");

  // Parse command-line arguments
  ntk::arg_parse(argc, argv);
  no_kinect = opt::no_kinect();

#ifdef OPENAL_SOUND_PLAYER
  // Initialize OpenAL for playing sounds.
  alutInit(&argc, argv);
  alGetError(); // TODO: check this!
#endif

  // Get base name
  char buffer[256];
  // gdb will supply the absolute path to the application in argv[0],
  // so we need to see if the path is absolute by seeing if it starts
  // with a forward slash.
  if (argc > 0 && argv[0][0] != '/') {
    getcwd(buffer, 256);
    dir_name = buffer;
    dir_name += "/";
  }
  dir_name += argv[0];
  dir_name.erase(dir_name.find_last_of("/") + 1);

  printf("Current directory is %s", dir_name.c_str());
  fflush(stdout);
}

int AppController::run() {
  // The drum set contains the drums and the sticks
  DrumSet drum_set;

  // The application window shows the different app screens.
  // NOTE: it's now instantiated as part of the AppController constructor.
  //AppViewer window;

  // The drum viewer displays the drum set
  DrumViewer viewer(&drum_set, &window);

  // Initialize the sound player.
  const char* INSTRUMENTS[] = {
    "xylophone/", "drums/" 
  };

  std::vector<std::vector<char*> > sound_prefixes;
  sound_prefixes.push_back(std::vector<char*>());
  sound_prefixes.push_back(std::vector<char*>());

  sound_prefixes[0].push_back("c");
  sound_prefixes[0].push_back("d");
  sound_prefixes[0].push_back("e");
  sound_prefixes[0].push_back("f");
  sound_prefixes[0].push_back("g");
  sound_prefixes[0].push_back("a");
  sound_prefixes[0].push_back("b");
  sound_prefixes[0].push_back("c2");
  
  sound_prefixes[1].push_back("cowbell");
  sound_prefixes[1].push_back("crash");
  sound_prefixes[1].push_back("hihatclosed");
  sound_prefixes[1].push_back("hihatopen");
  sound_prefixes[1].push_back("kick");
  sound_prefixes[1].push_back("ride");
  sound_prefixes[1].push_back("snare");
  sound_prefixes[1].push_back("tambourine");
  sound_prefixes[1].push_back("tomhigh");
  sound_prefixes[1].push_back("tomlow");
  sound_prefixes[1].push_back("triangle");
  
  const unsigned int INSTRUMENT_SIZES[] = {8,11};
  
  const char* SOUND_LOUDNESS_LEVELS[] = {"_soft", "_med", "_hard"};
  std::string base_dir = dir_name + "../../../";
  SoundFileFinder sff(base_dir.c_str(), INSTRUMENTS, sound_prefixes, INSTRUMENT_SIZES,
    SOUND_LOUDNESS_LEVELS, 3, 2, ".wav");

#ifdef APLAY_SOUND_PLAYER
  AplaySoundPlayer player(&sff);
#endif
#ifdef OSC_SOUND_PLAYER
  OscSoundPlayer player;
#endif
#ifdef OPENAL_SOUND_PLAYER
  OpenALSoundPlayer player(&sff);
#endif

#ifdef USE_KINECT_WII
  string kc_path(dir_name);
  kc_path += "../../kinect_calibration.yml";
  if (!no_kinect) {
    kcp =  new KinectController(kc_path.c_str());
  }
  WiiController wc;
  ChronosController cc;
  KinectWiiController kwc(kcp, &wc, &cc);
  DrumSetController dsc(&drum_set, &player, &viewer, &kwc);
  viewer.setKinect(kcp);
  viewer.setDrumSetController(&dsc);
#else
  DrumSetController(&drum_set, &player, &viewer, &viewer);
#endif

  // Add the layer to the window
  window.addLayer(&viewer);
  window.resize(640 * 2, 480 * 2);
  window.show();

  // Start the timer
  connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
  timer.start(1000 / 30);

  // Block until the app exits
  return exec();
}

void AppController::update() {
  // Repaint the viewer.
  window.update();

  // TODO: update other stuff.
}

AppController::~AppController() {
#ifdef OPENAL_SOUND_PLAYER
  // TODO: call this elsewhere (i.e. with atexit())?
  alutExit();
#endif
}
