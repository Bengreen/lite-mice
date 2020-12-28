// the setup function runs once when you press reset or power the board
#include "FastLED.h"

#define PIN09 9
#define PIXEL_PIN 10
#define NUM_LEDS 100

CRGB leds[NUM_LEDS];

const int delayTime=1;
const int maxSpr = 20;
const int numSpr = 8;
const int slowest = 30;

CRGB sprColour[maxSpr]={CRGB::Black};
int sprWaitTime[maxSpr]={1};
int sprMove[maxSpr]={1};
int sprPosition[maxSpr]={0};
int sprTime[maxSpr]={0};

 
CRGB mergeColour(int pos) {
  CRGB sumColour = CRGB::Black;
  for(int spr=0;spr<numSpr;++spr) {
    if(sprPosition[spr]== pos) {
      sumColour+=sprColour[spr];
    }
  }
  return sumColour;
}


void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN09, OUTPUT);
  FastLED.addLeds<NEOPIXEL, PIXEL_PIN>(leds, NUM_LEDS);

  for(int spr=0; spr<numSpr;++spr) {

    int brightness = random(255);
    sprWaitTime[spr]=slowest-(brightness*slowest/255-1);
    sprMove[spr]=1;
    sprTime[spr]=sprWaitTime[spr];
    sprColour[spr]=CHSV(random(255),random(255),brightness);
    sprPosition[spr]=0;
  }
  //Do all colours and positions first before setting LEDs
  for(int spr=0;spr<numSpr;++spr) {
    leds[sprPosition[spr]]=mergeColour(sprPosition[spr]);
  }
}

int ledState=LOW;


// the loop function runs over and over again forever
void loop() {
  for(int sprite=0; sprite<numSpr;++sprite) {
    if (sprTime[sprite]==0) {
      ledState=ledState==LOW ? HIGH : LOW;
      int oldPos = sprPosition[sprite];
      
      //leds[sprPosition[sprite]]-=sprColour[sprite]; //Sprite leaves this place
      if (sprPosition[sprite]+sprMove[sprite] >= NUM_LEDS) {
        sprPosition[sprite]=NUM_LEDS-1-(sprPosition[sprite]+sprMove[sprite]-(NUM_LEDS-1));
        sprMove[sprite]=-sprMove[sprite];
      } else if (sprPosition[sprite]+sprMove[sprite] <0) {
        sprPosition[sprite]=-(sprPosition[sprite]+sprMove[sprite]);
        sprMove[sprite]=-sprMove[sprite];
      } else {
        sprPosition[sprite]+=sprMove[sprite];
      }
      //Clean colour of old position
      leds[oldPos]=mergeColour(oldPos);
      //Set colour of new position
      leds[sprPosition[sprite]]=mergeColour(sprPosition[sprite]);
      //Set new wait time
      sprTime[sprite]=sprWaitTime[sprite];
    }
    sprTime[sprite]--;
  }
  
  FastLED.show();
  
  digitalWrite(LED_BUILTIN, ledState);   // turn the LED on (HIGH is the voltage level)
  //digitalWrite(PIN09, HIGH);
  //delay(delayTime);

  //digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  //digitalWrite(PIN09, LOW);
  delay(delayTime);
}
