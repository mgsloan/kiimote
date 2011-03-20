#include "chronos_controller.h"

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#endif

using namespace cv;
using namespace std;

void ChronosController::chronos_init() {
  //TODO: Break out run into seperate methods
}

ChronosController::ChronosController() : listener(NULL), chronos_history() {
  start();
}
void ChronosController::run() {

  // Commands to send to radio reciever
  char start[3]={0xff,0x07,0x03};
  char get[7]={0xff,0x08,0x07,0x00,0x00,0x00,0x00};
  char cmd[4]={0xff,0x06,0x04,0x00};
  char stop[6]={0xff,0x07,0x03,0xff,0x09,0x03};
  char buff[7]; 
  double zbuf[5];
  //lo_address t = lo_address_new("localhost", "6666");

  int i = 0;
  for(i = 0; i < 5; i++){
    zbuf[i] = 0;
  }

  struct termios settings;
  FILE * dongle;
  dongle = fopen("/dev/ttyACM0", "r+");
  if(dongle == NULL){
	return;
  }

  //settings.c_cflag = BAUD | CRTSCTS | DATABITS | STOPBITS | PARITYON | PARITY | CLOCAL (const) | CREAD (const) ;      
  settings.c_cflag = B115200 | CRTSCTS | CS8      | 0        | 0        | 0      | CLOCAL         | CREAD;
  settings.c_iflag = IGNPAR;
  settings.c_oflag = 0;
  settings.c_lflag = 0;       //ICANON;
  settings.c_cc[VMIN]=0;   // timeout = 1s
  settings.c_cc[VTIME]=10;
  tcflush(fileno(dongle), TCIFLUSH);
  tcsetattr(fileno(dongle), TCSANOW, &settings);
  fwrite(stop,1,3,dongle);
  fread(buff,1,3,dongle);  
  fwrite(stop,1,3,dongle);

  //Initialize History Array
  //TODO: Cleanup
  std::vector<double>* history = new std::vector<double>();
  for(int i = 0; i < HISTORY_SIZE; i++) {
    //    history->push_back(0.0);
  }

  double max_average = 0;
  while(1) {
    // Read sample from watch
    fwrite(get,1,7,dongle);
    fread(buff,1,7,dongle);
    if(buff[3]==1) {
      double z = (signed char)buff[6];
      zbuf[0] = zbuf[1];
      zbuf[1] = zbuf[2];
      zbuf[2] = zbuf[3];
      zbuf[3] = zbuf[4];
      zbuf[4] = z-10;
      if(zbuf[2] < 0 && zbuf[3] < 0 && zbuf[4] > 0){  
	float average = -1*(zbuf[2] + zbuf[3])/2;
	if(average > max_average){
	  max_average = average;
	}
	if(listener) {listener->chronosHit(average/max_average);}
	//TODO: Replace with Magnitude
      }
    }
  }
}



void ChronosController::setListener(IChronosListener* _listener) { listener = _listener; }


const std::vector<double>& ChronosController::getHistory() const {
  return chronos_history;
}
