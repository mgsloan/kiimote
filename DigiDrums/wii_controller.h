#ifndef WII_CONTROLLER
#define WII_CONTROLLER

#include "wiiuse.h"
#include <opencv2/core/core.hpp>
#include <QThread>

// TODO: do away with this constant!
#define MAX_wii_motes 2

class IWiiListener {
public:
  /** Called when a Wiimote is "hit". */
  virtual void wiiHit(int wmnum, float mag) = 0;

  /** Called on a button press. */
  virtual void wiiButtonPress(int wmnum, int button, bool release) = 0;
};

/**
 * Tracks Wiimotes by updating them at a regular timed interval.
 */
class WiiController : public QThread {
  Q_OBJECT

protected:
  IWiiListener* listener;
  wiimote** wii_motes;
  std::vector<int> wii_rumble;
  std::vector<std::vector<cv::Point3f>*> wii_history;
  //vector<HistoryFilter<cv::Point3f, PointMetric>> history;

  int wii_found, wii_connected;
  timeval tv;
  int usec;
  float max_downswing[2];

  int repeat_event; // which buttons to repeatedly fire.

  // Main loop.
  void run();
public:
  WiiController();

  int wiimotes_start();
  int search();

  void update();

  int countWiimotes();

  void handle_wii_event(struct wiimote_t* wm, int wmnum);
  void handle_disconnect(wiimote* wm);

  void vibrateWiimote(int wmnum);

  void setListener(IWiiListener* _listener);

  void setRepeat(int buttons);

  const std::vector<std::vector<cv::Point3f>*>& getHistory() const;
};

#endif // WII_CONTROLLER
