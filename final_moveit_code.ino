/* for the LEDS */
long randNumber;
#include <OctoWS2811.h>
const int ledsPerStrip = 15;
DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

/* for the accelerometer */

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

Adafruit_MMA8451 mma = Adafruit_MMA8451();

int points;
int rainbowColors[180];

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioPlaySdWav           playWav1;
// Use one of these 3 output types: Digital I2S, Digital S/PDIF, or Analog DAC

AudioOutputAnalog      audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy 3.5 & 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

//color-int
#define PINK 0
#define BLUE 1
#define GREEN 2
#define ORANGE 3
#define RED 4
#define BLANK 99

void setup() {
    //leds.begin();
    //leds.show();
    pinMode(1, OUTPUT);
    digitalWrite(1, HIGH);
    for (int i=0; i<180; i++) {
      int hue = i * 2;
      int saturation = 100;
      int lightness = 50;
      // pre-compute the 180 rainbow colors
      rainbowColors[i] = makeColor(hue, saturation, lightness);
    }
    digitalWrite(1, LOW);
    leds.begin();

    /* for the accelerometer */
    Serial.begin(9600);
    Serial.println("Adafruit MMA8451 test!");
    if (!mma.begin()) {
        Serial.println("Couldnt start");
        while (1);
    }
    Serial.println("MMA8451 found!");

    mma.setRange(MMA8451_RANGE_2_G);

    Serial.print("Range = ");
    Serial.print(2 << mma.getRange());
    Serial.println("G");
    pinMode(3, OUTPUT);

// audio 

    Serial.begin(9600);

    // Audio connections require memory to work.  For more
    // detailed information, see the MemoryAndCpuUsage example
    AudioMemory(8);

    // Comment these out if not using the audio adaptor board.
    // This may wait forever if the SDA & SCL pins lack
    // pullup resistors
    //sgtl5000_1.enable();
    //sgtl5000_1.volume(0.10);

    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    if (!(SD.begin(SDCARD_CS_PIN))) {
      // stop here, but print a message repetitively
      while (1) {
        Serial.println("Unable to access the SD card");
        delay(500);
      }
    }
    points = 0;
}

void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

  // A brief delay for the library read WAV info
  delay(5);

  // Simply wait for the file to finish playing.
  while (playWav1.isPlaying()) {
    // uncomment these lines if you audio shield
    // has the optional volume pot soldered
    //float vol = analogRead(15);
    //vol = vol / 1024;
    // sgtl5000_1.volume(vol);
  }
}

unsigned long startTime;

boolean End_game = false;

void loop() {

  if (End_game) {
    lightUpLEDs(BLANK); 
  }
  else {

    Serial.print("Current # of points: ");
    Serial.println(points);
    if (points%15 == 0 && points != 0){
      playFile("POMEG.WAV");
      for (int i = 0; i < ledsPerStrip; i++) {
        rainbow(10, 100);
      }
      points += 7;
      lightUpLEDs(BLANK);
    }
    else {
    
    int threshold = 4;
    
    /* generate random number 0-3 */
    randNumber = random(4);
    // clear lights
    lightUpLEDs(BLANK);
    /* Each number is correlated with light */
    lightUpLEDs(randNumber);
    boolean gotMotion = false;

    startTime = millis();
    
    while (!gotMotion && (millis() - startTime <= 3000)) {
      gotMotion = checkMotion(randNumber, threshold);
    }
    lightUpLEDs(BLANK); // turn off the LEDs

    if (!gotMotion) {
      digitalWrite(3, HIGH);  // turn on vibrating discs
      playFile("WRONG001.WAV");
      lightUpLEDs(RED);         // LEDs turn red
      delay(1500);            // wait 1.5 seconds
      digitalWrite(3, LOW);   // turn off vibrating discs
      lightUpLEDs(BLANK);
      if (points < 15) { 
       for (int i = 0; i < 4; i++) {
        leds.setPixel (i, colorFromInt(PINK));
        leds.show();
        }
        delay(5000);
      }
      else if (points > 14 && points < 30) {
       for (int i = 0; i < 8; i++) {
        leds.setPixel (i, colorFromInt(BLUE));
        leds.show();
        }
        delay(5000);
      }
     else if (points > 29 && points < 45) {
      for (int i = 0; i < 12; i++) {
      leds.setPixel (i, colorFromInt(GREEN));
      leds.show();
      }
      delay(5000);
     }
     else {
      for (int i = 0; i < ledsPerStrip; i++) {
      leds.setPixel (i, colorFromInt(ORANGE));
      leds.show();
      }
      delay(5000);
     }
    End_game = true;
    } 

    else {
      // they're correct!
      playFile("DING0001.WAV");  // filenames are always uppercase 8.3 format
      delay(500);
      points += 1;
      }
   }
}
}

boolean checkMotion(int motion, float threshold) {
  float init, current, diff;
  boolean condition;
  init = motionFromInt(motion);
  
  delay(100);

  current = motionFromInt(motion);
  diff = current - init;
  //Serial.print("init: ");
  //Serial.print(init);
  //Serial.print(" | current: ");
  //Serial.print(current);
  //Serial.print(" | diff: ");
  //Serial.println(diff);
  switch (motion) {
    case 0: condition = ((abs(diff) >= threshold) && current > 0); break;
    case 1: condition = ((abs(diff) >= threshold) &&  current < 0); break;
    case 2: condition = ((abs(diff) >= threshold) && current > 0); break;
    case 3: condition = ((abs(diff) >= threshold) && current < 0); break;
    default: condition = false; break;
  }

  return condition;
}

float motionFromInt(int motion) {
  mma.read();
  sensors_event_t event;
  mma.getEvent(&event);

  float val;
  
  switch (motion) {
    case PINK: val = event.acceleration.x; break; //right
    case GREEN: val = event.acceleration.x; break; //left
    case BLUE: val = event.acceleration.y; break; //up
    case ORANGE: val = event.acceleration.y; break; // down
    default: val = 0; break;
  }

  return val;
}

void lightUpLEDs(int color) {
  switch (color) {
    case PINK: 
      for (int i = 0; i < 4; i++) {
      leds.setPixel (i, colorFromInt(color));
      leds.show();
      delay(10);
      }
      break;
    case BLUE:
      for (int i = 4; i < 8; i++) {
      leds.setPixel (i, colorFromInt(color));
      leds.show();
      delay(10);
      }
      break;
    case GREEN:
      for (int i = 8; i < 12; i++) {
      leds.setPixel (i, colorFromInt(color));
      leds.show();
      delay(10);
      }
      break;
    case ORANGE:
      for (int i = 12; i < ledsPerStrip; i++) {
      leds.setPixel (i, colorFromInt(color));
      leds.show();
      delay(10);
      }
      break;
    case RED:
      for (int i = 0; i < ledsPerStrip; i++) {
        leds.setPixel (i, colorFromInt(color));
        leds.show();
        delay(10);
      }
      break;
    default:
      for (int i = 0; i < ledsPerStrip; i++) {
      leds.setPixel (i, colorFromInt(99));
      leds.show();
      delay(10);
      }
      break;
}
}

int colorFromInt(int color) {
  switch (color) {
    case BLUE: return 0x0000FF; // blue
    case GREEN: return 0x00FF00; // green
    case PINK: return 0xFF1088; // pink
    case ORANGE: return 0xE05800; // orange
    
    case RED: return 0xFF0000; // red
    default: return 0x000000; // black
  }
}

//Define rainbow // 


int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness)
{
  unsigned int red, green, blue;
  unsigned int var1, var2;

  if (hue > 359) hue = hue % 360;
  if (saturation > 100) saturation = 100;
  if (lightness > 100) lightness = 100;

  // algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
  if (saturation == 0) {
    red = green = blue = lightness * 255 / 100;
  } else {
    if (lightness < 50) {
      var2 = lightness * (100 + saturation);
    } else {
      var2 = ((lightness + saturation) * 100) - (saturation * lightness);
    }
    var1 = lightness * 200 - var2;
    red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
    green = h2rgb(var1, var2, hue) * 255 / 600000;
    blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
  }
  return (red << 16) | (green << 8) | blue;
}

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue)
{
  if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
  if (hue < 180) return v2 * 60;
  if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
  return v1 * 60;
}

void rainbow(int phaseShift, int cycleTime)
{
  int color, x, y, wait;

  wait = cycleTime * 1000 / ledsPerStrip;
  for (color=0; color < 180; color++) {
    digitalWrite(1, HIGH);
    for (x=0; x < ledsPerStrip; x++) {
      for (y=0; y < 8; y++) {
        int index = (color + x + y*phaseShift/2) % 180;
        leds.setPixel(x + y*ledsPerStrip, rainbowColors[index]);
      }
    }
    leds.show();
    digitalWrite(1, LOW);
    delayMicroseconds(wait);
  }
}
