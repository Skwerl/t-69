/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Libraries *////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

#include <Servo.h>
#include <Adafruit_PWMServoDriver.h>
#include "SwitchBT.h"
#include <usbhub.h>

/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Bluetooth Config */////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

USB Usb;
BTD Btd(&Usb);

// To Pair:
SwitchBT Switch(&Btd, PAIR);

// After Pair:
//SwitchBT Switch(&Btd);

bool bluetoothInit = false;

/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Servo Config */////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVO_FREQ 50
#define SERVO_OSC_FREQ 27000000

/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Head Left/Right *//////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

uint8_t headX_channel = 2;
float headX_min = 200;
float headX_max = 460;
float headX_center = headX_min + ((headX_max - headX_min) / 2);
float headX_target = headX_center;

/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Switch Parsing *///////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

byte* Report;

struct SwitchButtons {
  bool Idle;
  bool B_Button;
  bool A_Button;
  bool Y_Button;
  bool X_Button;
  bool L_Trigger;
  bool R_Trigger;
  bool ZL_Trigger;
  bool ZR_Trigger;
  bool StickL_Button;
  bool StickR_Button;
  bool Select_Button;
  bool Start_Button;
  bool Home_Button;
  bool Action_Button;
  int D_Pad;
  int Analog_StickL_X;
  int Analog_StickL_Y;
  int Analog_StickR_X;
  int Analog_StickR_Y;
};
SwitchButtons reset = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
SwitchButtons state = reset;

void handleEvent() {

  if (Report[0] == 0 && Report[1] == 0 && Report[2] == 8) {

    allButtonsReleased();
    state.Idle = 1;

  } else {

    state.Idle = 0;

    switch (Report[0]) {
      case 1:
        state.B_Button = 1;
        break;
      case 2:
        state.A_Button = 1;
        break;
      case 4:
        state.Y_Button = 1;
        break;
      case 8:
        state.X_Button = 1;
        break;
      case 16:
        state.L_Trigger = 1;
        break;
      case 32:
        state.R_Trigger = 1;
        break;
      case 64:
        state.ZL_Trigger = 1;
        break;
      case 128:
        state.ZR_Trigger = 1;
        break;
    }
    switch (Report[1]) {
      case 1:
        state.Select_Button = 1;
        break;
      case 2:
        state.Start_Button = 1;
        break;
      case 16:
        state.Home_Button = 1;
        break;
      case 32:
        state.Action_Button = 1;
        break;
      case 4:
        state.StickL_Button = 1;
        break;
      case 8:
        state.StickR_Button = 1;
        break;
    }
    switch (Report[2]) {
      case 8:
        state.D_Pad = 0;
        break;
      case 7:
        state.D_Pad = 45;
        break;
      case 0:
        state.D_Pad = 90;
        break;
      case 1:
        state.D_Pad = 135;
        break;
      case 2:
        state.D_Pad = 180;
        break;
      case 3:
        state.D_Pad = 225;
        break;
      case 4:
        state.D_Pad = 270;
        break;
      case 5:
        state.D_Pad = 315;
        break;
      case 6:
        state.D_Pad = 360;
        break;
    }

    // TODO: Parse analog stick inputs

  }

  if (!bluetoothInit) {
    if (state.Analog_StickL_X > 0) {
      return;
    } else {
      bluetoothInit = true;
      Serial.println("");
      Serial.println("Bluetooth Connection Established");
      wakeUp();
    }
  } else {

    /*
    Serial.print("IDL");
    Serial.print(state.Idle);
    Serial.print(".");
    Serial.print("B");
    Serial.print(state.B_Button);
    Serial.print(".");
    Serial.print("A");
    Serial.print(state.A_Button);
    Serial.print(".");
    Serial.print("Y");
    Serial.print(state.Y_Button);
    Serial.print(".");
    Serial.print("X");
    Serial.print(state.X_Button);
    Serial.print(".");
    Serial.print("L");
    Serial.print(state.L_Trigger);
    Serial.print(".");
    Serial.print("R");
    Serial.print(state.R_Trigger);
    Serial.print(".");
    Serial.print("ZL");
    Serial.print(state.ZL_Trigger);
    Serial.print(".");
    Serial.print("ZR");
    Serial.print(state.ZR_Trigger);
    Serial.print(".");
    Serial.print("SEL");
    Serial.print(state.Select_Button);
    Serial.print(".");
    Serial.print("STR");
    Serial.print(state.Start_Button);
    Serial.print(".");
    Serial.print("ACK");
    Serial.print(state.Action_Button);
    Serial.print(".");
    Serial.print("HOM");
    Serial.print(state.Home_Button);
    Serial.print(".");
    Serial.print("DPAD");
    Serial.print(state.D_Pad);
    Serial.print(".");
    Serial.print("STKL");
    Serial.print(state.StickL_Button);
    Serial.print(".");
    Serial.print("STKR");
    Serial.print(state.StickR_Button);
    Serial.print(".");
    Serial.println("");
    */

    doAction();

  }

}

void allButtonsReleased() {
  state = reset;
}

/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* T-69 Logic *///////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

#define SERIAL_BAUD 115200

boolean driveN = false;
boolean driveS = false;
boolean driveE = false;
boolean driveW = false;

void doAction(void) {

  driveS = false;
  driveN = false;
  driveE = false;
  driveW = false;

  if (state.D_Pad > 0) {
    //if (state.D_Pad >= 315 || state.D_Pad <= 045) { driveS = true; }
    if (state.D_Pad == 360) {
      driveW = true;
    }
    //if (state.D_Pad >= 135 && state.D_Pad <= 225) { driveN = true; }
    if (state.D_Pad == 180) {
      driveE = true;
    }
  }

  if (driveW) { headX_target++; }
  if (driveE) { headX_target--; }

  if (headX_target <= headX_min) { headX_target = headX_min; }
  if (headX_target >= headX_max) { headX_target = headX_max; }

  if (!driveE && !driveW) { headX_target = headX_center; }

  if (state.Action_Button) {
    restDrive();
  }

  sendMove();

}

void restDrive(void) {
  //Serial.println("restDrive");
}

void sendMove(void) {
  //Serial.println(headX_target);
  pwm.setPWM(headX_channel, 0, headX_target);
}

void wakeUp() {
  Serial.println("wakeUp");
}

/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Arduino Runtime *//////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void setup(void) {

  // PWM, Servo
  pwm.begin();
  pwm.setOscillatorFrequency(SERVO_OSC_FREQ);
  pwm.setPWMFreq(SERVO_FREQ);

  // Bluetooth
  Serial.begin(SERIAL_BAUD);
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nSwitch Bluetooth Library Started"));

}

void loop(void) {

  Usb.Task();

  if (Switch.connected()) {

    Report = Switch.Report;
    handleEvent();

  }

}
