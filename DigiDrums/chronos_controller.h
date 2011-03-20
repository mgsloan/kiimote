#ifndef CHRONOS_CONTROLLER
#define CHRONOS_CONTROLLER

#define HISTORY_SIZE 3

#include "lo/lo.h"
#include "termios.h"
#include <QThread>
#include <opencv2/core/core.hpp>

class IChronosListener {
 public:
  virtual void chronosHit(float mag) = 0;
};

class ChronosController : public QThread {
  Q_OBJECT

protected:
  IChronosListener* listener;
  std::vector<double> chronos_history;
  int chronos_connected;

  void run();

public:
  ChronosController();
  
  void chronos_init();

  void setListener(IChronosListener* _listener);

  const std::vector<double>& getHistory() const;
  
};

#endif
