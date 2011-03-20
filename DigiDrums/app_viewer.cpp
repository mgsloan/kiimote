#define GLEW_STATIC
#include <GL/glew.h>

#include "app_viewer.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "wiiuse.h"
#include <algorithm>
#include <QPainter>
#include <QFont>
#include <QPen>
using namespace std;

// IButtonListener
void IButtonListener::buttonPressed(int button, bool released) {}

// IMouseListener
void IMouseListener::mouseMoved(int x, int y) {}
void IMouseListener::mousePressed(int button) {}
void IMouseListener::mouseLeft() {}

AppViewer::AppViewer() : QGLWidget(), button_listener(NULL),
  mouse_listener(NULL) {
  setMouseTracking(true);
}

QSize AppViewer::getSurfaceSize() { return size(); }

/**
 * Translate keyboard presses to Wiimote keys.
 */
int translateKey(int key) {
  switch(key) {
    case Qt::Key_Left:
      return WIIMOTE_BUTTON_LEFT;
    case Qt::Key_Right:
      return WIIMOTE_BUTTON_RIGHT;
    case Qt::Key_Up:
      return WIIMOTE_BUTTON_UP;
    case Qt::Key_Down:
      return WIIMOTE_BUTTON_DOWN;
    case Qt::Key_A:
      return WIIMOTE_BUTTON_A;
    case Qt::Key_Plus:
      return WIIMOTE_BUTTON_PLUS;
    case Qt::Key_Minus:
      return WIIMOTE_BUTTON_MINUS;
    case Qt::Key_Home:
      return WIIMOTE_BUTTON_HOME;
    case Qt::Key_1:
      return WIIMOTE_BUTTON_ONE;
    case Qt::Key_2:
      return WIIMOTE_BUTTON_TWO;
  }
  return 0;
}

void AppViewer::keyPressEvent(QKeyEvent *e) {
  int wiimote_button = translateKey(e->key());
  if (wiimote_button && button_listener) {
    button_listener->buttonPressed(wiimote_button, false);
  }
}

void AppViewer::keyReleaseEvent(QKeyEvent *e) {
  int wiimote_button = translateKey(e->key());
  if (wiimote_button && button_listener) {
    button_listener->buttonPressed(wiimote_button, true);
  }
}


void AppViewer::mouseMoveEvent(QMouseEvent *e) {
  // TODO: make mouse coordination configurable
  last_mouse_pos = QVector2D(e->x(), e->y());
  if (mouse_listener) {
    mouse_listener->mouseMoved(e->x(), e->y());
  }
}

void AppViewer::mousePressEvent(QMouseEvent *e) {
  if (mouse_listener) {
    mouse_listener->mousePressed(e->button());
  }
}

void AppViewer::leaveEvent(QEvent *e) {
  if (mouse_listener) {
    mouse_listener->mouseLeft();
  }
}

void AppViewer::initializeGL() {
  // TODO: should stuff go here?  The individual screens are responsible for
  // sharing this OpenGL context, and initializing themselves on paintGL().
  GLenum err = glewInit();
  if (GLEW_OK == err) {
    printf("Got glew!!\n");
  }
}

void AppViewer::resizeGL(int w, int h) {
  // Resize the viewport
  glViewport(0,0,w,h);
}

void AppViewer::paintGL() {
  // Draw the visible layers.
  for (vector<IScreenLayer*>::iterator iter = layers.begin();
      iter != layers.end(); iter++) {
    (*iter)->paintGL();
  }
}

void AppViewer::setButtonListener(IButtonListener* listener) {
  button_listener = listener;
}

void AppViewer::setMouseListener(IMouseListener* listener) {
  mouse_listener = listener;
}

void AppViewer::addLayer(IScreenLayer* layer) {
  layers.push_back(layer);
  layer->bindScreenManager(this);
}

void AppViewer::removeLayer(IScreenLayer* layer) {
  for (vector<IScreenLayer*>::iterator iter = layers.begin();
      iter != layers.end(); iter++) {
    if (*iter == layer) {
      layers.erase(iter);
      break;
    }
  }
}

QSize AppViewer::getScreenSize() { return size(); }
