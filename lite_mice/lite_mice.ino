// the setup function runs once when you press reset or power the board
#include "FastLED.h"

#define PIXEL_PIN 10
#define WHEEL_PIN A3
#define LED0_PIN 6
#define LED1_PIN 5
#define BUTTON0_PIN 7
#define BUTTON1_PIN 8

#define NUM_LEDS 100

CRGB leds[NUM_LEDS];

const int delayTime = 1;
const int maxMice = 20;
int mouseTrainStart = 0;
int mouseTrainEnd = 0;
const byte maxTime = 30;

struct LiteMouse
{
  CRGB colour;
  byte waitTime;
  short step;
  byte time;
  int position;
};

LiteMouse mice[maxMice] = {
    {
        .colour = CRGB::Black,
        .waitTime = 1,
        .step = 1,
        .time = 0,
        .position = 0,
    }};

unsigned long debounceConfirmTime = 0;
unsigned long stateStartTime = 0;
const unsigned long debounceDelay = 50;
const unsigned long longPress = 500;

int hue = 255;
int sat = 255;
int magicVal = -1; // Default to magic on

enum buttonState
{
  button0 = 0b10,
  button1 = 0b01,
  allHeld = 0b00,
  released = 0b11
};

int buttonsState = 3;               // the initial state
int buttonsPrevious = buttonsState; // Initial state

CRGB mergeMouseColour(int pos)
{
  CRGB sumColour = CRGB::Black;
  for (int mouse = mouseTrainStart; mouse != mouseTrainEnd; mouse = (mouse + 1) % maxMice)
  {
    if (mice[mouse].position == pos)
    {
      sumColour += mice[mouse].colour;
    }
  }
  return sumColour;
}

void setMouse(int mouse, int hue, int sat = 255, int magicVal = -1)
{

  int val = (magicVal == -1) ? random(255) : magicVal;
  Serial.print("New Mouse: ");
  Serial.print(mouse);
  Serial.print(" = (");
  Serial.print(hue);
  Serial.print(", ");
  Serial.print(sat);
  Serial.print(", ");
  Serial.print(val);
  Serial.print(magicVal == -1 ? " MAGIC" : " SET");
  Serial.println(")");
  byte time = byte(maxTime - (val * maxTime / 255 - 1));

  mice[mouse] = {
      .colour = CHSV(hue, sat, val),
      .waitTime = time,
      .step = 1,
      .time = time,
      .position = 0,
  };
  Serial.println("updated mouse");
}

/*
 * Settings:
 * Handmade = /dev/ttyUSB0
 * UNO = /dev/
 */

void setup()
{
  Serial.begin(9600);
  Serial.println("Booted....");
  Serial.println(sizeof(struct LiteMouse));
  Serial.println(sizeof(CRGB));
  Serial.println(sizeof(mice));

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT_PULLUP);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(WHEEL_PIN, INPUT); // Configured by the analogInput

  FastLED.addLeds<NEOPIXEL, PIXEL_PIN>(leds, NUM_LEDS);

  Serial.println("Booted.");
}

// the loop function runs over and over again forever
void loop()
{
  // Serial.print("[");
  // Serial.print(mouseTrainStart);
  // Serial.print(", ");
  // Serial.print(mouseTrainEnd);
  // Serial.println("]");
  for (int mouse = mouseTrainStart; mouse != mouseTrainEnd; mouse = (mouse + 1) % maxMice)
  {
    // Serial.print("Checking mouse ");
    // Serial.println(mouse);
    if (mice[mouse].time == 0)
    {
      // Serial.print("Moving mouse ");
      // Serial.print(mouse);
      int oldPos = mice[mouse].position;
      // Serial.print(" from ");
      // Serial.println(oldPos);
      if (mice[mouse].position + mice[mouse].step >= NUM_LEDS)
      {
        // Serial.print("END OF TRACK ");
        mice[mouse].position = NUM_LEDS - 1 - (mice[mouse].position + mice[mouse].step - (NUM_LEDS - 1));
        mice[mouse].step = -mice[mouse].step;
      }
      else if (mice[mouse].position + mice[mouse].step < 0)
      {
        // Serial.print("START OF TRACK ");
        mice[mouse].position = -(mice[mouse].position + mice[mouse].step);
        mice[mouse].step = -mice[mouse].step;
      }
      else
      {
        mice[mouse].position += mice[mouse].step;
        // Serial.println("Moving to ");
      }
      // Serial.print(mice[mouse].position);
      // Serial.print("-");
      // Serial.println(mice[mouse].step);
      //Clean colour of old position
      leds[oldPos] = mergeMouseColour(oldPos);
      //Set colour of new position
      leds[mice[mouse].position] = mergeMouseColour(mice[mouse].position);
      //Set new wait time
      mice[mouse].time = mice[mouse].waitTime;
    }
    mice[mouse].time--;
  }

  FastLED.show();

  int wheelRead = analogRead(WHEEL_PIN);
  int buttons = (digitalRead(BUTTON1_PIN) << 1) | digitalRead(BUTTON0_PIN);
  unsigned long tnow = millis();

  if (buttons != buttonsPrevious)
  {
    debounceConfirmTime = millis() + debounceDelay;
    buttonsPrevious = buttons;
  }
  else if (tnow > debounceConfirmTime)
  {
    if (buttons != buttonsState)
    {

      // Do button state change action here
      Serial.print("Buttons state changed ");
      Serial.println(buttons);
      switch (buttons)
      {
      // Leading edge
      case button0:
        Serial.println("Button 0");
        break;
      case button1:
        Serial.println("Button 1");
        break;
      case allHeld:
        Serial.println("Both");
        break;
      case released: // Trailing edge
        Serial.print("Held for ");
        Serial.print(tnow - stateStartTime);
        Serial.print(", release from ");
        Serial.println(buttonsState);
        if ((tnow - stateStartTime) > longPress)
        {
          Serial.println("LONGPRESS");
          switch (buttonsState)
          {
          case button0:
            sat = wheelRead / 4;
            Serial.print("Sat = ");
            Serial.println(sat);
            break;
          case button1:
            magicVal = wheelRead / 4;
            Serial.print("Val = ");
            Serial.println(magicVal);
            break;
          case allHeld:
            Serial.println("Magic (Random Val)");
            magicVal = -1;
            break;
          }
        }
        else
        {
          Serial.println("SHORT PRESS");

          switch (buttonsState)
          {
          case button0:
          {
            int newMouseTrainEnd = (mouseTrainEnd + 1) % maxMice;
            if (newMouseTrainEnd != mouseTrainStart)
            {
              hue = wheelRead / 4;
              setMouse(mouseTrainEnd, hue, sat, magicVal);
              mouseTrainEnd = newMouseTrainEnd;
              Serial.print("end = ");
              Serial.println(mouseTrainEnd);
            }
            else
            {
              Serial.println("MAX MICE");
            }
          }
          break;
          case button1:

            if (mouseTrainStart != mouseTrainEnd)
            {
              int oldPosition = mice[mouseTrainStart].position;

              mouseTrainStart = (mouseTrainStart + 1) % maxMice;
              leds[oldPosition] = mergeMouseColour(oldPosition);
              Serial.print("start = ");
              Serial.println(mouseTrainStart);
            }
            else
            {
              Serial.println("NO MICE");
            }
            break;
          case allHeld:
            Serial.println("Small Magic");
            break;
          }
        }
        break;
      }
      stateStartTime = tnow;
      buttonsState = buttons;
    }
  }

  analogWrite(LED0_PIN, wheelRead / 4);
  analogWrite(LED1_PIN, 255 - wheelRead / 4);

  delay(delayTime);
}
