#ifndef APP_VIEWER_H
#define APP_VIEWER_H
#include <stdio.h>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QVector3D>
#include <QVector2D>
#include <vector>
#include "idrumsticks.h"
#include "sound_player.h"
#include "drumset.h"
#include <QSize>

/**
 * Classes implementing this interface can be shown on the App Viewer.
 */
class IScreenManager;
class IScreenLayer {
public:
  /**
   * Draws this layer's contents to the screen.
   * Remember to leave the context in the state it was in before!
   */
  virtual void paintGL() = 0;

  // TODO: do we need unbindScreenManager?
  virtual void bindScreenManager(IScreenManager* manager) = 0;
};

/** Screen manager interface. */
class IScreenManager {
public:
  /** Add a layer to the screen manager. */
  virtual void addLayer(IScreenLayer* layer) = 0;

  /** Remove the given layer from the screen manager. */
  virtual void removeLayer(IScreenLayer* layer) = 0;

  virtual QSize getScreenSize() = 0;
};

/**
 * Classes implementing this interface can receive button input events
 * from classes implementing IButtonInput.
 *
 * This allows the menu screen to listen for button input events that
 * come from the keyboard or either Wiimote.  IDrumstickListener would
 * also provide the stick number, which the menu screen isn't interested in.
 */
class IButtonListener {
public:
  /**
   * Called when a button is pressed or released.
   * @param button The Wiimote button code
   * @param released True if the button was released rather than pressed.
   */
  virtual void buttonPressed(int button, bool released);
};

/**
 * Classes implementing this interface can provide button input
 * events to other classes.
 */
class IButtonInput {
public:
  /** Set the button listener. */
  virtual void setButtonListener(IButtonListener* listener) = 0;
};

/**
 * Classes implementing this interface can receive mouse input events from
 * classes implementing IMouseInput.
 */
class IMouseListener {
public:
  /** Called when the mouse changes position. */
  virtual void mouseMoved(int x, int y);

  /** Called when the mouse is pressed. */
  virtual void mousePressed(int button);

  /** Called when the mouse leaves the region. */
  virtual void mouseLeft();
};

class IMouseInput {
public:
  /** Registers a mouse listener. */
  virtual void setMouseListener(IMouseListener* listener) = 0;

  /** Gets the size of the window. */
  virtual QSize getSurfaceSize() = 0;
};

/**
 * The application window.
 */
class AppViewer : public QGLWidget, public IButtonInput, public IMouseInput,
  public IScreenManager {
  Q_OBJECT

public:
  AppViewer();

  // IButtonListener
  void setButtonListener(IButtonListener* listener);

  // IMouseListener
  void setMouseListener(IMouseListener* listener);
  QSize getSurfaceSize();

  // IScreenManager
  void addLayer(IScreenLayer* layer);
  QSize getScreenSize();
  void removeLayer(IScreenLayer* layer);

protected:
  // Layers to display
  std::vector<IScreenLayer*> layers;

  // Fields for the fake drumsticks
  QVector2D last_mouse_pos;

  // Button listener
  IButtonListener* button_listener;

  // Mouse listener
  IMouseListener* mouse_listener;

  // OpenGL entry points
  void initializeGL();
  void resizeGL( int w, int h );
  void paintGL();

  // Keyboard and mouse event handlers
  void keyPressEvent( QKeyEvent *e );
  void keyReleaseEvent( QKeyEvent *e );
  void mouseMoveEvent( QMouseEvent *e );
  void mousePressEvent(QMouseEvent *e);
  void leaveEvent(QEvent *e);
};

#endif // APP_VIEWER_H
