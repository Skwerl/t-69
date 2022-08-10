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
///////////////////////* Head Movement *////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

uint8_t headX_channel = 2;
float headX_min = 200;
float headX_max = 460;
float headX_center = headX_min + ((headX_max - headX_min) / 2);

uint8_t headY_channel = 7;
float headY_min = 200;
float headY_max = 400;
float headY_center = headY_min + ((headY_max - headY_min) / 2);

uint8_t headR_channel = 6;
float headR_min = 200;
float headR_max = 460;
float headR_center = headR_min + ((headR_max - headR_min) / 2);
float headR_target = headR_center;

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

// Define stick signal ranges:
float Analog_StickL_X_Min = 40;
float Analog_StickL_X_Max = 225;
float Analog_StickL_Y_Min = 20;
float Analog_StickL_Y_Max = 216;
float Analog_StickR_X_Min = 48;
float Analog_StickR_X_Max = 220;
float Analog_StickR_Y_Min = 32;
float Analog_StickR_Y_Max = 210;

// Define stick dead zones:
float Analog_Stick_X_DZ_Min = 120;
float Analog_Stick_X_DZ_Max = 145;
float Analog_Stick_Y_DZ_Min = 118;
float Analog_Stick_Y_DZ_Max = 130;

void handleEvent() {

  allButtonsReleased();

  state.Analog_StickL_X = constrain(Report[3], Analog_StickL_X_Min, Analog_StickL_X_Max);
  state.Analog_StickL_Y = constrain(Report[4], Analog_StickL_Y_Min, Analog_StickL_Y_Max);
  state.Analog_StickR_X = constrain(Report[5], Analog_StickR_X_Min, Analog_StickR_X_Max);
  state.Analog_StickR_Y = constrain(Report[6], Analog_StickR_Y_Min, Analog_StickR_Y_Max);

  bool Idle_Sticks = true;

  if (state.Analog_StickL_X >= Analog_Stick_X_DZ_Min && state.Analog_StickL_X <= Analog_Stick_X_DZ_Max) {
    state.Analog_StickL_X = 0;
  } else {
    Idle_Sticks = false;
  }

  if (state.Analog_StickL_Y >= Analog_Stick_Y_DZ_Min && state.Analog_StickL_Y <= Analog_Stick_Y_DZ_Max) {
    state.Analog_StickL_Y = 0;
  } else {
    Idle_Sticks = false;
  }

  if (state.Analog_StickR_X >= Analog_Stick_X_DZ_Min && state.Analog_StickR_X <= Analog_Stick_X_DZ_Max) {
    state.Analog_StickR_X = 0;
  } else {
    Idle_Sticks = false;
  }

  if (state.Analog_StickR_Y >= Analog_Stick_Y_DZ_Min && state.Analog_StickR_Y <= Analog_Stick_Y_DZ_Max) {
    state.Analog_StickR_Y = 0;
  } else {
    Idle_Sticks = false;
  }

  if (Report[0] == 0 && Report[1] == 0 && Report[2] == 8 && Idle_Sticks) {
    state.Idle = 1;
  }

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
    case 48:
      state.L_Trigger = 1;
      state.R_Trigger = 1;
      break;
    case 192:
      state.ZL_Trigger = 1;
      state.ZR_Trigger = 1;
      break;
    case 80:
      state.L_Trigger = 1;
      state.ZL_Trigger = 1;
      break;
    case 160:
      state.R_Trigger = 1;
      state.ZR_Trigger = 1;
      break;
    case 240:
      state.L_Trigger = 1;
      state.R_Trigger = 1;
      state.ZL_Trigger = 1;
      state.ZR_Trigger = 1;
      break;
    case 3:
      state.B_Button = 1;
      state.A_Button = 1;
      break;
    case 5:
      state.Y_Button = 1;
      state.B_Button = 1;
      break;
    case 10:
      state.X_Button = 1;
      state.A_Button = 1;
      break;
    case 12:
      state.Y_Button = 1;
      state.X_Button = 1;
      break;
    case 17:
      state.L_Trigger = 1;
      state.B_Button = 1;
      break;
    case 18:
      state.L_Trigger = 1;
      state.A_Button = 1;
      break;
    case 20:
      state.L_Trigger = 1;
      state.Y_Button = 1;
      break;
    case 24:
      state.L_Trigger = 1;
      state.X_Button = 1;
      break;
    case 33:
      state.R_Trigger = 1;
      state.B_Button = 1;
      break;
    case 34:
      state.R_Trigger = 1;
      state.A_Button = 1;
      break;
    case 36:
      state.R_Trigger = 1;
      state.Y_Button = 1;
      break;
    case 40:
      state.R_Trigger = 1;
      state.X_Button = 1;
      break;
    case 65:
      state.ZL_Trigger = 1;
      state.B_Button = 1;
      break;
    case 66:
      state.ZL_Trigger = 1;
      state.A_Button = 1;
      break;
    case 68:
      state.ZL_Trigger = 1;
      state.Y_Button = 1;
      break;
    case 72:
      state.ZL_Trigger = 1;
      state.X_Button = 1;
      break;
    case 129:
      state.ZR_Trigger = 1;
      state.B_Button = 1;
      break;
    case 130:
      state.ZR_Trigger = 1;
      state.A_Button = 1;
      break;
    case 132:
      state.ZR_Trigger = 1;
      state.Y_Button = 1;
      break;
    case 136:
      state.ZR_Trigger = 1;
      state.X_Button = 1;
      break;
    case 49:
      state.L_Trigger = 1;
      state.R_Trigger = 1;
      state.B_Button = 1;
      break;
    case 50:
      state.L_Trigger = 1;
      state.R_Trigger = 1;
      state.A_Button = 1;
      break;
    case 52:
      state.L_Trigger = 1;
      state.R_Trigger = 1;
      state.Y_Button = 1;
      break;
    case 56:
      state.L_Trigger = 1;
      state.R_Trigger = 1;
      state.X_Button = 1;
      break;
    case 81:
      state.L_Trigger = 1;
      state.ZL_Trigger = 1;
      state.B_Button = 1;
      break;
    case 82:
      state.L_Trigger = 1;
      state.ZL_Trigger = 1;
      state.A_Button = 1;
      break;
    case 84:
      state.L_Trigger = 1;
      state.ZL_Trigger = 1;
      state.Y_Button = 1;
      break;
    case 88:
      state.L_Trigger = 1;
      state.ZL_Trigger = 1;
      state.X_Button = 1;
      break;
    case 161:
      state.R_Trigger = 1;
      state.ZR_Trigger = 1;
      state.B_Button = 1;
      break;
    case 162:
      state.R_Trigger = 1;
      state.ZR_Trigger = 1;
      state.A_Button = 1;
      break;
    case 164:
      state.R_Trigger = 1;
      state.ZR_Trigger = 1;
      state.Y_Button = 1;
      break;
    case 168:
      state.R_Trigger = 1;
      state.ZR_Trigger = 1;
      state.X_Button = 1;
      break;
    case 193:
      state.ZL_Trigger = 1;
      state.ZR_Trigger = 1;
      state.B_Button = 1;
      break;
    case 194:
      state.ZL_Trigger = 1;
      state.ZR_Trigger = 1;
      state.A_Button = 1;
      break;
    case 196:
      state.ZL_Trigger = 1;
      state.ZR_Trigger = 1;
      state.Y_Button = 1;
      break;
    case 200:
      state.ZL_Trigger = 1;
      state.ZR_Trigger = 1;
      state.X_Button = 1;
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

  if (!bluetoothInit) {
    if (state.Idle == 0) {
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
    Serial.print("ANA1`");
    Serial.print(state.Analog_StickL_X);
    Serial.print(",");
    Serial.print(state.Analog_StickL_Y);
    Serial.print(".");
    Serial.print("ANA2`");
    Serial.print(state.Analog_StickR_X);
    Serial.print(",");
    Serial.print(state.Analog_StickR_Y);
    Serial.print(".");
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

void doAction(void) {

  if (state.Action_Button) {
    restDrive();
  }

  // Calculate head movement:
  float headX_target = headX_center;
  float headY_target = headY_center;
  if (state.Analog_StickR_X > 0) {
    headX_target = map(state.Analog_StickR_X, Analog_StickR_X_Min, Analog_StickR_X_Max, headX_min, headX_max);
  }
  if (state.Analog_StickR_Y > 0) {
    headY_target = map(state.Analog_StickR_Y, Analog_StickR_Y_Max, Analog_StickR_Y_Min, headY_min, headY_max);
  }

  // Calculate head rotation:
  bool rotateHeadL = state.L_Trigger;
  bool rotateHeadR = state.R_Trigger;
  if (rotateHeadL) { headR_target++; }
  if (rotateHeadR) { headR_target--; }
  headR_target = constrain(headR_target, headR_min, headR_max);
  if (!rotateHeadL && !rotateHeadR) { headR_target = headR_center; }
  //Serial.println(headR_target);

  // Send movements:

  pwm.setPWM(headX_channel, 0, headX_target);
  pwm.setPWM(headY_channel, 0, headY_target);
  //pwm.setPWM(headR_channel, 0, headR_target);

}

void restDrive(void) {
  Serial.println("restDrive");
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
