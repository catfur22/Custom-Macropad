
#include <Mouse.h>
#include <Keyboard.h>

const int JOY_X = D0;
const int JOY_Y = D1;
const int JOY_SW = D2;

const int buttons[8] = {
  D3, D4, D5, D6,
  D7, D8, D9, D10
};

enum ButtonAction {
  PAN_LEFT,
  PAN_RIGHT,
  PAN_UP,
  PAN_DOWN,
  ZOOM_IN,
  ZOOM_OUT,
  ORBIT_LEFT,
  ORBIT_RIGHT
};

const int DEADZONE = 350;
const int MAX_MOUSE_SPEED = 7;

const int PAN_SPEED = 5;
const int ZOOM_SPEED = 1;
const int ORBIT_BUTTON_SPEED = 4;

const unsigned long BUTTON_INTERVAL = 45;

// Joystick center calibration
int centerX = 2048;
int centerY = 2048;


bool buttonState[8] = {false};
unsigned long lastButtonAction[8] = {0};

bool mouseMiddleHeld = false;
bool shiftHeld = false;

void setNavigationMode(bool orbit) {
  if (orbit) {
    if (!mouseMiddleHeld) {
      Mouse.press(MOUSE_MIDDLE);
      mouseMiddleHeld = true;
    }

    if (!shiftHeld) {
      Keyboard.press(KEY_LEFT_SHIFT);
      shiftHeld = true;
    }
  } else {
    if (shiftHeld) {
      Keyboard.release(KEY_LEFT_SHIFT);
      shiftHeld = false;
    }

    if (mouseMiddleHeld) {
      Mouse.release(MOUSE_MIDDLE);
      mouseMiddleHeld = false;
    }
  }
}

void releaseMouseControls() {
  setNavigationMode(false);
}

void calibrateJoystick() {
  long totalX = 0;
  long totalY = 0;

  for (int i = 0; i < 100; i++) {
    totalX += analogRead(JOY_X);
    totalY += analogRead(JOY_Y);
    delay(10);
  }

  centerX = totalX / 100;
  centerY = totalY / 100;
}

int joystickMovement(int value, int center) {
  int difference = value - center;

  if (abs(difference) < DEADZONE) {
    return 0;
  }

  int movement = map(
    difference,
    -2048, 2048,
    -MAX_MOUSE_SPEED, MAX_MOUSE_SPEED
  );

  return constrain(
    movement,
    -MAX_MOUSE_SPEED,
    MAX_MOUSE_SPEED
  );
}

void handleJoystick() {
  int xValue = analogRead(JOY_X);
  int yValue = analogRead(JOY_Y);

  int moveX = joystickMovement(xValue, centerX);
  int moveY = joystickMovement(yValue, centerY);

  moveY = -moveY;

  if (moveX != 0 || moveY != 0) {
    setNavigationMode(true);
    Mouse.move(moveX, moveY, 0);
  } else {
    releaseMouseControls();
  }
}

void handleButton(int index) {
  unsigned long now = millis();

  if (now - lastButtonAction[index] < BUTTON_INTERVAL) {
    return;
  }

  lastButtonAction[index] = now;

  switch (index) {

    case PAN_LEFT:
      setNavigationMode(false);
      Mouse.press(MOUSE_MIDDLE);
      Mouse.move(-PAN_SPEED, 0, 0);
      Mouse.release(MOUSE_MIDDLE);
      break;

    case PAN_RIGHT:
      setNavigationMode(false);
      Mouse.press(MOUSE_MIDDLE);
      Mouse.move(PAN_SPEED, 0, 0);
      Mouse.release(MOUSE_MIDDLE);
      break;

    case PAN_UP:
      setNavigationMode(false);
      Mouse.press(MOUSE_MIDDLE);
      Mouse.move(0, -PAN_SPEED, 0);
      Mouse.release(MOUSE_MIDDLE);
      break;

    case PAN_DOWN:
      setNavigationMode(false);
      Mouse.press(MOUSE_MIDDLE);
      Mouse.move(0, PAN_SPEED, 0);
      Mouse.release(MOUSE_MIDDLE);
      break;

    case ZOOM_IN:
      Mouse.move(0, 0, ZOOM_SPEED);
      break;

    case ZOOM_OUT:
      Mouse.move(0, 0, -ZOOM_SPEED);
      break;

    case ORBIT_LEFT:
      setNavigationMode(true);
      Mouse.move(-ORBIT_BUTTON_SPEED, 0, 0);
      releaseMouseControls();
      break;

    case ORBIT_RIGHT:
      setNavigationMode(true);
      Mouse.move(ORBIT_BUTTON_SPEED, 0, 0);
      releaseMouseControls();
      break;
  }
}

void setup() {
  analogReadResolution(12);

  pinMode(JOY_SW, INPUT_PULLUP);

  for (int i = 0; i < 8; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  calibrateJoystick();

  Mouse.begin();
  Keyboard.begin();

  delay(500);
}

void loop() {

  handleJoystick();

  for (int i = 0; i < 8; i++) {
    bool pressed = (digitalRead(buttons[i]) == LOW);

    if (pressed) {
      handleButton(i);
    }
  }

  delay(5);
}
