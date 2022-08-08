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
  bool U_Button;
  bool D_Button;
  bool L_Button;
  bool R_Button;
  bool Y_Button;
  bool X_Button;
  bool B_Button;
  bool A_Button;
  bool SL_Button;
  bool SR_Button;
  bool L_Trigger;
  bool ZL_Trigger;
  bool R_Trigger;
  bool ZR_Trigger;
  bool Stick_Button;
  bool Select_Button;
  bool Action_Button;
  int Analog_Stick;
};
SwitchButtons reset = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
SwitchButtons state = reset;

void handleEvent() {

  if (Report[0] == 0 && Report[1] == 0 && Report[2] == 8) {

    allButtonsReleased();
    state.Idle = 1;

  } else {

    state.Idle = 0;

    switch (Report[0]) {
      case 4:
        state.U_Button = 1;
        state.B_Button = 1;
        break;
      case 2:
        state.D_Button = 1;
        state.X_Button = 1;
        break;
      case 1:
        state.L_Button = 1;
        state.A_Button = 1;
        break;
      case 8:
        state.R_Button = 1;
        state.Y_Button = 1;
        break;
      case 16:
        state.SL_Button = 1;
        break;
      case 32:
        state.SR_Button = 1;
        break;
    }
    switch (Report[1]) {
      case 64:
        state.L_Trigger = 1;
        state.R_Trigger = 1;
        break;
      case 128:
        state.ZL_Trigger = 1;
        state.ZR_Trigger = 1;
        break;
      case 1:
      case 2:
        state.Select_Button = 1;
        break;
      case 32:
        state.Action_Button = 1;
        break;
      case 4:
      case 8:
        state.Stick_Button = 1;
        break;
    }
    switch (Report[2]) {
      case 8:
        state.Analog_Stick = 0;
        break;
      case 7:
        state.Analog_Stick = 45;
        break;
      case 0:
        state.Analog_Stick = 90;
        break;
      case 1:
        state.Analog_Stick = 135;
        break;
      case 2:
        state.Analog_Stick = 180;
        break;
      case 3:
        state.Analog_Stick = 225;
        break;
      case 4:
        state.Analog_Stick = 270;
        break;
      case 5:
        state.Analog_Stick = 315;
        break;
      case 6:
        state.Analog_Stick = 360;
        break;
    }

  }

  if (!bluetoothInit) {
    if (state.Analog_Stick > 0) {
      return;
    } else {
      bluetoothInit = true;
      Serial.println("");
      Serial.println("Bluetooth Connection Established");
      wakeUp();
    }
  } else {

    Serial.print("IDL");
    Serial.print(state.Idle);
    Serial.print(".");
    Serial.print("UBT");
    Serial.print(state.U_Button);
    Serial.print(".");
    Serial.print("DBT");
    Serial.print(state.D_Button);
    Serial.print(".");
    Serial.print("LBT");
    Serial.print(state.L_Button);
    Serial.print(".");
    Serial.print("RBT");
    Serial.print(state.R_Button);
    Serial.print(".");
    Serial.print("SLB");
    Serial.print(state.SL_Button);
    Serial.print(".");
    Serial.print("SRB");
    Serial.print(state.SR_Button);
    Serial.print(".");
    Serial.print("LTR");
    Serial.print(state.L_Trigger);
    Serial.print(".");
    Serial.print("ZLT");
    Serial.print(state.ZL_Trigger);
    Serial.print(".");
    Serial.print("STK");
    Serial.print(state.Stick_Button);
    Serial.print(".");
    Serial.print("SEL");
    Serial.print(state.Select_Button);
    Serial.print(".");
    Serial.print("ACK");
    Serial.print(state.Action_Button);
    Serial.print(".");
    Serial.print("ANA");
    Serial.print(state.Analog_Stick);
    Serial.println("");

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

  driveS = state.U_Button;
  driveN = state.D_Button;
  driveE = state.L_Button;
  driveW = state.R_Button;

  if (state.Analog_Stick > 0) {
    //if (state.Analog_Stick >= 315 || state.Analog_Stick <= 045) { driveS = true; }
    if (state.Analog_Stick == 360) {
      driveW = true;
    }
    //if (state.Analog_Stick >= 135 && state.Analog_Stick <= 225) { driveN = true; }
    if (state.Analog_Stick == 180) {
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
