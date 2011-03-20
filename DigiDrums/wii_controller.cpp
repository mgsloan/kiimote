#include "wii_controller.h"

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifndef WIN32
#include <unistd.h>
#endif

using namespace cv;
using namespace std;

/**
 *    @brief Callback that handles an event.
 *
 *    @param wm        Pointer to a wiimote_t structure.
 *
 *    This function is called automatically by the wiiuse library when an
 *    event occurs on the specified wiimote.
 */
void WiiController::handle_wii_event(struct wiimote_t* wm, int wmnum) {
  // Scan for pressed buttons
  // TODO: the fact that we have to do this could be a deficiency in my
  // Wiimote interface.  It might be better to register a list of buttons to
  // listen to.
  if (listener) {
    unsigned int just_pressed = wm->btns ^ wm->btns_held;
    for (unsigned int i = 1; i != 0; i <<= 1) {
      if ((i & just_pressed)) {
        listener->wiiButtonPress(wmnum, i, false);
      }
      if (i & wm->btns_released || ((i & repeat_event)
                                 && (i & wm->btns_held))) {
        listener->wiiButtonPress(wmnum, i, true);
      }
    }
  }

    
    if (WIIUSE_USING_ACC(wm)) {
        
        cv::Point3f downswing_avg(0.0, 0.0, 0.0);
        cv::Point3f upswing_avg(0.0, 0.0, 0.0);
        for(int i = 0; i < 6; i++){
            if(i < 4) {
                downswing_avg += (*wii_history[wmnum])[i];
            } else {
                upswing_avg += (*wii_history[wmnum])[i];
            }
        }
        downswing_avg *= (1 / 4.0);
        upswing_avg *= (1 / 2.0);
        
        wii_history[wmnum]->erase(wii_history[wmnum]->begin());
        wii_history[wmnum]->push_back(
            cv::Point3_<float>(wm->gforce.x, wm->gforce.y, wm->gforce.z));
        vector<cv::Point3f>::iterator h = wii_history[wmnum]->begin();
        cv::Point3f h1 = *(h++), h2 = *(h++), h3 = *(h++);
        cv::Point3f h4 = *(h++), h5 = *(h++), h6 = *(h++);
        if(h1.z < 0 && h2.z < 0 && h3.z < 0 &&
           h4.z < 0 && h5.z > 0 && h6.z > 0) {
	      if(-downswing_avg.z > max_downswing[wmnum]) {
	        max_downswing[wmnum] = -downswing_avg.z;
          }
	      float intensity = -downswing_avg.z/max_downswing[wmnum];
          if (listener) { listener->wiiHit(wmnum, intensity); }
          //wii_rumble[wmnum] = 1;
        }
        /*
        if((h1.x < 0.5 && h2.x < 0.5 && h3.x < 0.5 &&
            h4.x < 0.5 && h5.x > 0.5 && h6.x > 0.5) ||
           (h1.x < -0.5 && h2.x < -0.5 && h3.x < -0.5 &&
            h4.x < -0.5 && h5.x > -0.5 && h6.x > -0.5)) {
          //TODO: Should we treat the intensity of these hits differently?
          if(-downswing_avg.z > max_downswing[wmnum]) {
            max_downswing[wmnum] = -downswing_avg.z;
	      }
          float intensity = -downswing_avg.z/max_downswing[wmnum];
          if (listener) { listener->wiiHit(wmnum, intensity); }
          //wii_rumble[wmnum] = 1;
        }
        */
    }
}

/**
 *    @brief Callback that handles a disconnection event.
 *
 *    @param wm                Pointer to a wiimote_t structure.
 *
 *    This can happen if the POWER button is pressed, or
 *    if the connection is interrupted.
 */
void WiiController::handle_disconnect(wiimote* wm) {
    printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
}


/*
void test(struct wiimote_t* wm, byte* data, unsigned short len) {
    printf("test: %i [%x %x %x %x]\n", len, data[0], data[1], data[2], data[3]);
}
*/

int WiiController::countWiimotes() { return wii_connected; }

void WiiController::vibrateWiimote(int wmnum) {
  // TODO: does this work?
  wii_rumble[wmnum] = 1;
}

int WiiController::search() {
    //This will return the number of actual wii_motes that are in discovery mode.
    wii_found = wiiuse_find(wii_motes, MAX_wii_motes, 5); // 5 = time delay.
    if (!wii_found) {
        printf ("No wii_motes found.");
        return 0;
    }

    //This will return the number of established connections to the found wii_motes.
    wii_connected = wiiuse_connect(wii_motes, MAX_wii_motes);
    /*
    for (int i = 0; i < wii_connected; i++) {
        wii_history.push_back(list<cv::Point3f>());
    } */
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
    for (int i = 0; i < min(4, wii_connected); i++)
        wiiuse_set_leds(wii_motes[0], WIIMOTE_LED_1 * (1 << i));

    for (int i = 0; i < wii_connected; i++) {
        wiiuse_rumble(wii_motes[i], 1);
        wiiuse_motion_sensing( wii_motes[i], 1);
    }

    sleep_ms(200);

    for (int i = 0; i < wii_connected; i++)
        wiiuse_rumble(wii_motes[i], 0);
    return wii_connected;
}

/**
 *    @brief main()
 *
 *    Connect to up to two wii_motes and print any events
 *    that occur on either device.
 */
void WiiController::run() {
    gettimeofday(&tv, NULL);
    usec = tv.tv_usec;
    for(int j = 0; j < 2; j++){    
        //TODO:cleanup memory
        vector<Point3f>* history = new vector<Point3f>();
        for(int i = 0; i < 6; i++){
            history->push_back(Point3f(0,0,0));
        }
        wii_history.push_back(history);
        wii_rumble.push_back(0);
    }

    //Initialize an array of wiimote objects.
    wii_motes = wiiuse_init(MAX_wii_motes);

    search();

    /*
     *    Maybe I'm interested in the battery power of the 0th
     *    wiimote.  This should be WIIMOTE_ID_1 but to be sure
     *    you can get the wiimote assoicated with WIIMOTE_ID_1
     *    using the wiiuse_get_by_id() function.
     *
     *    A status request will return other things too, like
     *    if any expansions are plugged into the wiimote or
     *    what LEDs are lit.
     */
    //wiiuse_status(wii_motes[0]);


    /*
     *    This is the main loop
     *
     *    wiiuse_poll() needs to be called with the wiimote array
     *    and the number of wiimote structures in that array
     *    (it doesn't matter if some of those wii_motes are not used
     *    or are not connected).
     *
     *    This function will set the event flag for each wiimote
     *    when the wiimote has things to report.
     */
    printf("before main loop\n");
    while (1) {
        if (!wii_connected) {
            search();
        } else if (wiiuse_poll(wii_motes, MAX_wii_motes)) {
            int i = 0;
            for (; i < MAX_wii_motes; ++i) {
                switch (wii_motes[i]->event) {
                    case WIIUSE_EVENT:
                        /* a generic event occured */
                        handle_wii_event(wii_motes[i], i);
                        break;

                    case WIIUSE_STATUS:
                        /* a status event occured */
//                        handle_ctrl_status(wii_motes[i]);
                        break;

                    case WIIUSE_DISCONNECT:
                    case WIIUSE_UNEXPECTED_DISCONNECT:
                        /* the wiimote disconnected */
                        handle_disconnect(wii_motes[i]);
                        break;

                    case WIIUSE_READ_DATA:
                        /*
                         *    Data we requested to read was returned.
                         *    Takwmnume a look at wii_motes[i]->read_req
                         *    for the data.
                         */
                        break;

                    default:
                        break;
                }
            }
            if(wii_rumble[0] == 1){
                wiiuse_rumble(wii_motes[0], wii_rumble[0]);
                usec = tv.tv_usec;
            }
            if(wii_rumble[1] == 1){
                wiiuse_rumble(wii_motes[1], wii_rumble[1]);
                usec = tv.tv_usec;
            }
            wii_rumble[0] = 0;
            wii_rumble[1] = 0;
            //usleep(20000);
            gettimeofday(&tv, NULL);
            if(tv.tv_usec > (usec + 55000) || tv.tv_usec < usec){
                wiiuse_rumble(wii_motes[0], 0);

                if (wii_connected > 1)
                    wiiuse_rumble(wii_motes[1], 0);    
            }
        }
        
        
    }

    /*
     *    Disconnect the wii_motes
     */
    // TODO: handle this in a better way!
    wiiuse_cleanup(wii_motes, MAX_wii_motes);
}


WiiController::WiiController() : listener(NULL), wii_history() {
  // Start the Wiimote main thread.
  repeat_event = WIIMOTE_BUTTON_UP | WIIMOTE_BUTTON_RIGHT | WIIMOTE_BUTTON_B;
  start();
}

void WiiController::setListener(IWiiListener* _listener) { listener = _listener; }


const vector<vector<cv::Point3f>*>& WiiController::getHistory() const {
  return wii_history;
}

void WiiController::setRepeat(int buttons) {
  repeat_event = buttons;
}
