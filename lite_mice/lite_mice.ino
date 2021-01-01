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
const int maxSpr = 20;
int numSpr = 8;
const int slowest = 30;

CRGB sprColour[maxSpr] = {CRGB::Black};
int sprWaitTime[maxSpr] = {1};
int sprMove[maxSpr] = {1};
int sprPosition[maxSpr] = {0};
int sprTime[maxSpr] = {0};

unsigned long debounceConfirmTime = 0;
unsigned long stateStart = 0;
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

CRGB mergeColour(int pos)
{
  CRGB sumColour = CRGB::Black;
  for (int spr = 0; spr < numSpr; ++spr)
  {
    if (sprPosition[spr] == pos)
    {
      sumColour += sprColour[spr];
    }
  }
  return sumColour;
}

void setSprite(int spr, int hue, int sat = 255, int magicVal = -1)
{
  Serial.print("New Mouse: ");
  Serial.print(spr);
  Serial.print(" = (");
  Serial.print(hue);
  Serial.print(", ");
  Serial.print(sat);
  Serial.print(", ");
  Serial.print(magicVal);
  Serial.println(")");

  int val = (magicVal == -1) ? random(255) : magicVal;

  sprWaitTime[spr] = slowest - (val * slowest / 255 - 1);
  sprMove[spr] = 1;
  sprTime[spr] = sprWaitTime[spr];
  sprColour[spr] = CHSV(hue, sat, val);
  sprPosition[spr] = 0;
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
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // pinMode(PIN09, OUTPUT);
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT_PULLUP);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(WHEEL_PIN, INPUT); // Configured by the analogInput

  FastLED.addLeds<NEOPIXEL, PIXEL_PIN>(leds, NUM_LEDS);

  for (int spr = 0; spr < numSpr; ++spr)
  {
    setSprite(spr, random(255));
  }
  //Do all colours and positions first before setting LEDs
  for (int spr = 0; spr < numSpr; ++spr)
  {
    leds[sprPosition[spr]] = mergeColour(sprPosition[spr]);
  }
  Serial.println("Booted.");
}

int ledState = LOW;

// the loop function runs over and over again forever
void loop()
{
  for (int sprite = 0; sprite < numSpr; ++sprite)
  {
    if (sprTime[sprite] == 0)
    {
      ledState = ledState == LOW ? HIGH : LOW;
      int oldPos = sprPosition[sprite];

      //leds[sprPosition[sprite]]-=sprColour[sprite]; //Sprite leaves this place
      if (sprPosition[sprite] + sprMove[sprite] >= NUM_LEDS)
      {
        sprPosition[sprite] = NUM_LEDS - 1 - (sprPosition[sprite] + sprMove[sprite] - (NUM_LEDS - 1));
        sprMove[sprite] = -sprMove[sprite];
      }
      else if (sprPosition[sprite] + sprMove[sprite] < 0)
      {
        sprPosition[sprite] = -(sprPosition[sprite] + sprMove[sprite]);
        sprMove[sprite] = -sprMove[sprite];
      }
      else
      {
        sprPosition[sprite] += sprMove[sprite];
      }
      //Clean colour of old position
      leds[oldPos] = mergeColour(oldPos);
      //Set colour of new position
      leds[sprPosition[sprite]] = mergeColour(sprPosition[sprite]);
      //Set new wait time
      sprTime[sprite] = sprWaitTime[sprite];
    }
    sprTime[sprite]--;
  }

  FastLED.show();

  digitalWrite(LED_BUILTIN, ledState); // turn the LED on (HIGH is the voltage level)
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
        Serial.print(tnow - stateStart);
        Serial.print(", release from ");
        Serial.println(buttonsState);
        if ((tnow - stateStart) > longPress)
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
            Serial.print("Value");
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
            if (numSpr < maxSpr)
            {
              hue = wheelRead / 4;
              setSprite(numSpr, hue, sat, magicVal);
              ++numSpr;
            }
            else
            {
              Serial.println("Max Mice");
            }
            break;
          case button1:
            if (numSpr > 0)
            {
              --numSpr;
              Serial.print("Deleting Mouse: ");
              Serial.println(numSpr);
            }
            else
            {
              Serial.println("No Mice");
            }
            break;
          case allHeld:
            Serial.println("Small Magic");
            break;
          }
        }
        break;
      }
      stateStart = tnow;
      buttonsState = buttons;
    }
  }

  analogWrite(LED0_PIN, wheelRead / 4);
  analogWrite(LED1_PIN, 255 - wheelRead / 4);

  delay(delayTime);
}
