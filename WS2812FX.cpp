/*
  WS2812FX.cpp - Library for WS2812 LED effects.

  Harm Aldick - 2016
  www.aldick.org


  FEATURES
    * A lot of blinken modes and counting
    * WS2812FX can be used as drop-in replacement for Adafruit Neopixel Library

  NOTES
    * Uses the Adafruit Neopixel library. Get it here:
      https://github.com/adafruit/Adafruit_NeoPixel



  LICENSE

  The MIT License (MIT)

  Copyright (c) 2016  Harm Aldick

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  CHANGELOG

  2016-05-28   Initial beta release
  2016-06-03   Code cleanup, minor improvements, new modes
  2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
  2017-02-02   added external trigger functionality (e.g. for sound-to-light)
  2017-02-02   removed "blackout" on mode, speed or color-change
  2017-09-26   implemented segment and reverse features
  2017-11-16   changed speed calc, reduced memory footprint
  2018-02-24   added hooks for user created custom effects
*/

#include "WS2812FX.h"

/*
 * ColorPalettes
 */

// A mostly red palette with green accents and white trim.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM = {  
  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
  CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray, 
  CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green 
};

// A mostly (dark) green palette with red berries.
#define Holly_Green 0x00580c
#define Holly_Red   0xB00402
const TProgmemRGBPalette16 Holly_p FL_PROGMEM = {  
  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
  Holly_Green, Holly_Green, Holly_Green, Holly_Red 
};

// A red and white striped palette
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM = {  
  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
  CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
  CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray 
};
// A mostly blue palette with white accents.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM = {  
  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
  CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray 
};

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM = {  
  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, 
  HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight, 
  QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight, 
  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight 
};

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM = {  
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0xE0F0FF 
};

// A palette reminiscent of large 'old-school' C9-size tree lights
// in the five classic colors: red, orange, green, blue, and white.
#define C9_Red    0xB80400
#define C9_Orange 0x902C02
#define C9_Green  0x046002
#define C9_Blue   0x070758
#define C9_White  0x606820
const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM = {  
  C9_Red,    C9_Orange, C9_Red,    C9_Orange,
  C9_Orange, C9_Red,    C9_Orange, C9_Red,
  C9_Green,  C9_Green,  C9_Green,  C9_Green,
  C9_Blue,   C9_Blue,   C9_Blue,
  C9_White
};

// A cold, icy pale blue palette
#define Ice_Blue1 0x0C1040
#define Ice_Blue2 0x182080
#define Ice_Blue3 0x5080C0
const TProgmemRGBPalette16 Ice_p FL_PROGMEM = {
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3
};

// Iced Colors
const TProgmemRGBPalette16 Ice_Colors_p FL_PROGMEM =  {
  CRGB::Black, CRGB::Black, CRGB::Blue,  CRGB::Blue,
  CRGB::Blue,  CRGB::Blue,  CRGB::Blue,  CRGB::Aqua,
  CRGB::Aqua,  CRGB::Aqua,  CRGB::Aqua,  CRGB::Aqua,
  CRGB::Aqua,  CRGB::White, CRGB::White, CRGB::White
};

// Totally Black palette (for fade through black transitions)
const TProgmemRGBPalette16 Total_Black_p FL_PROGMEM = {
  CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
  CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
  CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
  CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
};

// Shades
#define SHADE01 0xF0
#define SHADE02 0x80
#define SHADE03 0x40
#define SHADE04 0x20
#define SHADE05 0x10
// Values
#define REDVAL(A)   ((A << 16)& 0xff0000)
#define GREENVAL(A) ((A <<  8)& 0x00ff00)
#define BLUEVAL(A)  ((A <<  0)& 0x0000ff)

// Shades of Red
const TProgmemRGBPalette16 Shades_Of_Red_p FL_PROGMEM = {
  REDVAL(SHADE01), REDVAL(SHADE02), REDVAL(SHADE03), REDVAL(SHADE04),
  REDVAL(SHADE05), CRGB::Black,     CRGB::Black,     REDVAL(SHADE04),
  REDVAL(SHADE03), REDVAL(SHADE02), REDVAL(SHADE01), CRGB::Black,
  CRGB::Black,     REDVAL(SHADE02), REDVAL(SHADE03), CRGB::Black
};

// Shades of Green
const TProgmemRGBPalette16 Shades_Of_Green_p FL_PROGMEM = {
  GREENVAL(SHADE01), GREENVAL(SHADE02), GREENVAL(SHADE03), GREENVAL(SHADE04),
  GREENVAL(SHADE05), CRGB::Black,       CRGB::Black,       GREENVAL(SHADE04),
  GREENVAL(SHADE03), GREENVAL(SHADE02), GREENVAL(SHADE01), CRGB::Black,
  CRGB::Black,       GREENVAL(SHADE02), GREENVAL(SHADE03), CRGB::Black
};

// Shades of Blue
const TProgmemRGBPalette16 Shades_Of_Blue_p FL_PROGMEM = {
  BLUEVAL(SHADE01), BLUEVAL(SHADE02), BLUEVAL(SHADE03), BLUEVAL(SHADE04),
  BLUEVAL(SHADE05), CRGB::Black,      CRGB::Black,      BLUEVAL(SHADE04),
  BLUEVAL(SHADE03), BLUEVAL(SHADE02), BLUEVAL(SHADE01), CRGB::Black,
  CRGB::Black,      BLUEVAL(SHADE02), BLUEVAL(SHADE03), CRGB::Black
};



/*
 * <Begin> Service routines
 */

// Not much to be initialized...
void WS2812FX::init() {
  RESET_RUNTIME;            // this should be the only occurrence of RESET_RUNTIME now... 
  FastLED.clear(true);      // During init, all pixels should be black.
  FastLED.show();           // We show once to write the Led data.
}

/*
 * the overall service task. To be called as often as possible / useful
 * (at least at the desired frame rate)
 * --> see STRIP_MAX_FPS
 */
void WS2812FX::service() {
  if(_running || _triggered) {
    unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
    // work through teh segments...
    for(uint8_t i=0; i < _num_segments; i++) {
      _segment_index = i;
      // this is ratherfrom the initial library.
      // There are barely any tasks controlled by return value.
      // So they usually return the STRIP_MIN_DELAY
      if(now > SEGMENT_RUNTIME.next_time || _triggered) {
        //uint16_t delay = 
        (this->*_mode[SEGMENT.mode])();
        SEGMENT_RUNTIME.next_time = now + STRIP_MIN_DELAY; //(int)delay;
      }
      // check if we fade to a new FX mode.
      if(_transition)
      {
        EVERY_N_MILLISECONDS(8)
        {
          nblend(_bleds, leds, SEGMENT_LENGTH, _blend);
          _blend = qadd8(_blend,1);
        }
        if(_blend == 255)
        {
          _transition = false;
          _blend = 0;
        }
      }
      else
      {
        EVERY_N_MILLISECONDS(10)
        {
          fadeToBlackBy(_bleds, SEGMENT_LENGTH, 4);
        }
        nblend(_bleds, leds, SEGMENT_LENGTH, SEGMENT.blur);
      }
    }
    // Write the data
    FastLED.show();


    // Every huetime we increase the baseHue by the respective deltaHue.
    // set deltahue to 0, to turn this off.
    //EVERY_N_MILLISECONDS(SEGMENT.hueTime)
    if(now > SEGMENT_RUNTIME.nextHue)
    {
      if(SEGMENT.reverse)
        SEGMENT_RUNTIME.baseHue -= SEGMENT.deltaHue;
      else
        SEGMENT_RUNTIME.baseHue += SEGMENT.deltaHue;
        
      SEGMENT_RUNTIME.nextHue = now + SEGMENT.hueTime*10;
    }

    // Palette fading / blending
    EVERY_N_MILLISECONDS(16) { // Blend towards the target palette
      nblendPaletteTowardPalette(_currentPalette, _targetPalette, 16);
      if(_currentPalette == _targetPalette)
      {
        _currentPaletteName = _targetPaletteName;
      }
    }

    // Autoplay
    //EVERY_N_SECONDS(_segments[0].autoplayDuration)
    if(now > SEGMENT_RUNTIME.nextAuto)
    {
      if(_segments[0].autoplay && !_transition)
      {
        if(_segments[0].mode == (getModeCount()-1))
        {
          setMode(0);
        }
        else
        {
          setMode(_segments[0].mode+1);
        }
        SEGMENT_RUNTIME.nextAuto = now + SEGMENT.autoplayDuration*1000;
      }
    }

    if(now > SEGMENT_RUNTIME.nextPalette)
    {
      if(SEGMENT.autoPal && !_transition)
      {
        if(getTargetPaletteNumber() >= getPalCount()-1)
        {
          setTargetPalette(0);
        }
        else
        {
          setTargetPalette(getTargetPaletteNumber()+1);
        }
        SEGMENT_RUNTIME.nextPalette = now + SEGMENT.autoPalDuration*1000;
      }       
    }

    // reset trigger...
    _triggered = false;
  }
}

void WS2812FX::start() {
  _running = true;
}

void WS2812FX::stop() {
  _running = false;
  strip_off();
}

void WS2812FX::trigger() {
  _triggered = true;
}

void WS2812FX::show() {
  nblend(_bleds, leds, SEGMENT_LENGTH, SEGMENT.blur);
  FastLED.show();
}

/*
 * <End> Service routines
 */


/* 
 * <Begin> Helper Functions
 */


/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit triwave
 */
inline uint16_t WS2812FX::triwave16(uint16_t in) {
  if(in & 0x8000)
  {
    in = 65535 - in;
  }
  return in << 1;
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit quadwave
 */
inline uint16_t WS2812FX::quadwave16(uint16_t in) {
   return ease16InOutQuad( triwave16( in));
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit easeInOutQuad
 */
inline uint16_t WS2812FX::ease16InOutQuad( uint16_t i) {
    uint16_t j = i;
    if( j & 0x8000 ) {
        j = 65535 - j;
    }
    uint16_t jj  = scale16(  j, j);
    uint16_t jj2 = jj << 1;
    if( i & 0x8000 ) {
        jj2 = 65535 - jj2;
    }
    return jj2;
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit cubicWave
 */
inline uint16_t WS2812FX::cubicwave16(uint16_t in) {
    return ease16InOutCubic( triwave16( in));
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit easeInOutCubic
 */
inline uint16_t WS2812FX::ease16InOutCubic( uint16_t i) {
    
    uint16_t ii  = scale16(  i, i);
    uint16_t iii = scale16( ii, i);

    uint32_t r1 = (3 * (uint16_t)(ii)) - ( 2 * (uint16_t)(iii));

    uint16_t result = r1;

    // if we got "65536", return 65535:
    if( r1 & 0x10000) {
        result = 65535;
    }
    return result;
}


void WS2812FX::drawFractionalBar(int pos16, int width, const CRGBPalette16 &pal, uint8_t cindex, uint8_t max_bright = 255) {
  // Draw a "Fractional Bar" of light starting at position 'pos16', which is counted in
  // sixteenths of a pixel from the start of the strip.  Fractional positions are
  // rendered using 'anti-aliasing' of pixel brightness.
  // The bar width is specified in whole pixels.
  // Arguably, this is the interesting code.
  int i = pos16 / 16; // convert from pos to raw pixel number
  
  uint8_t frac = pos16 & 0x0F; // extract the 'factional' part of the position
 
  // brightness of the first pixel in the bar is 1.0 - (fractional part of position)
  // e.g., if the light bar starts drawing at pixel "57.9", then
  // pixel #57 should only be lit at 10% brightness, because only 1/10th of it
  // is "in" the light bar:
  //
  //                       57.9 . . . . . . . . . . . . . . . . . 61.9
  //                        v                                      v
  //  ---+---56----+---57----+---58----+---59----+---60----+---61----+---62---->
  //     |         |        X|XXXXXXXXX|XXXXXXXXX|XXXXXXXXX|XXXXXXXX |  
  //  ---+---------+---------+---------+---------+---------+---------+--------->
  //                   10%       100%      100%      100%      90%        
  //
  // the fraction we get is in 64ths. We subtract from 255 because we want a high
  // fraction (e.g. 0.9) to turn into a low brightness (e.g. 0.1)
  uint8_t firstpixelbrightness = 255 - (frac*16);//map8(15 - (frac), 0, max_bright);
 
  // if the bar is of integer length, the last pixel's brightness is the
  // reverse of the first pixel's; see illustration above.
  uint8_t lastpixelbrightness  = 255 - firstpixelbrightness; //map8(15 - firstpixelbrightness, 0, max_bright);
 
  // For a bar of width "N", the code has to consider "N+1" pixel positions,
  // which is why the "<= width" below instead of "< width".
  uint8_t bright;
  for( int n = 0; n <= width; n++) {
    if(n == 0) {
      // first pixel in the bar
      bright = firstpixelbrightness;
    } else if( n == width ) {
      // last pixel in the bar
      bright = lastpixelbrightness;
    } else {
      // middle pixels
      bright = max_bright;
    }
 
    CRGB newColor;
    if(i<=SEGMENT.stop && i >= SEGMENT.start)
    {
      newColor = leds[i] | ColorFromPalette(pal, cindex, bright, SEGMENT.blendType); 
      // we blend based on the "baseBeat"
      nblend(leds[i], newColor, qadd8(SEGMENT.beat88>>8, 24));
    }
    i++;
  }
}

/*
 * Returns a new, random wheel index with a minimum distance of 42 from pos.
 */
uint8_t WS2812FX::get_random_wheel_index(uint8_t pos) {
  uint8_t r = 0;
  uint8_t d = 0;

  while(d < 42) {
    r = random8(255);
    d = min(abs(pos - r), 255 - abs(pos - r));
  }

  return r;
}

/*
 * Turns everything off. Doh.
 */
void WS2812FX::strip_off() {
  _running = false;
  FastLED.clear();
}

/*
 * Add sparks
 */
void WS2812FX::addSparks(uint8_t probability = 10, bool onBlackOnly = true, bool white = false) {
  if(random8(probability) != 0) return;

  uint16_t pos = random16(SEGMENT.start, SEGMENT.stop); // Pick an LED at random.
  
  if(leds[pos] && onBlackOnly) return;

  if(white)
  {
    leds[pos] += CRGB(0xffffff);
  }
  else
  {
    leds[pos] += ColorFromPalette(_currentPalette, random8(SEGMENT_RUNTIME.baseHue,SEGMENT_RUNTIME.baseHue+64), random8(92,255), SEGMENT.blendType);
  }
  return;
}

void WS2812FX::map_pixels_palette(uint8_t *hues, uint8_t bright = 255, TBlendType blend = LINEARBLEND) { 
  for(uint16_t i = 0; i<SEGMENT_LENGTH; i++)
  {
    leds[i + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[i], bright, blend);
  }
  return;
}


CRGB WS2812FX::computeOneTwinkle( uint32_t ms, uint8_t salt) {
  //  This function takes a time in pseudo-milliseconds,
  //  figures out brightness = f( time ), and also hue = f( time )
  //  The 'low digits' of the millisecond time are used as 
  //  input to the brightness wave function.  
  //  The 'high digits' are used to select a color, so that the color
  //  does not change over the course of the fade-in, fade-out
  //  of one cycle of the brightness wave function.
  //  The 'high digits' are also used to determine whether this pixel
  //  should light at all during this cycle, based on the TWINKLE_DENSITY.
  //  uint8_t TWINKLE_SPEED = _twinkleSpeed; //map8(SEGMENT.beat88>>8, 2, 8);
  //  Overall twinkle density.
  //  0 (NONE lit) to 8 (ALL lit at once).  
  //  Default is 5.
  //  #define TWINKLE_DENSITY _twinkleDensity //6

  uint16_t ticks = ms >> (8-_segments[0].twinkleSpeed);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);
  
  uint8_t bright = 0;
  if( ((slowcycle8 & 0x0E)/2) < _segments[0].twinkleDensity) {
    bright = attackDecayWave8( fastcycle8);
  }

  #define COOL_LIKE_INCANDESCENT 0

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if( bright > 0) {
    c = ColorFromPalette( _currentPalette, hue, bright, SEGMENT.blendType);
    if( COOL_LIKE_INCANDESCENT == 1 ) {
      coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}


uint8_t WS2812FX::attackDecayWave8( uint8_t i) {
  if( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i/2));
  }
}


void WS2812FX::coolLikeIncandescent( CRGB& c, uint8_t phase) {
  /* 
  This function is like 'triwave8', which produces a 
  symmetrical up-and-down triangle sawtooth waveform, except that this
  function produces a triangle wave with a faster attack and a slower decay:
  
      / \ 
     /     \ 
    /         \ 
   /             \ 
  
  This function takes a pixel, and if its in the 'fading down'
  part of the cycle, it adjusts the color a little bit like the 
  way that incandescent bulbs fade toward 'red' as they dim. */
  if( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}
/*
 * End Twinklle Fox
 */

uint16_t WS2812FX::pride(bool glitter = false) {
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  //uint8_t sat8 = beatsin88(SEGMENT.beat88/12 + 1, 220, 250);// beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( SEGMENT.beat88/3 + 1, 96, 224); //beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( SEGMENT.beat88/5+1, (25 * 256), (40 * 256)); //beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(SEGMENT.beat88/7+1, 23, 60);//beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(SEGMENT.beat88/9+1, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( (SEGMENT.beat88/5)*2+1, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < SEGMENT_LENGTH; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = ColorFromPalette(_currentPalette, hue8, bri8, SEGMENT.blendType); //CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (SEGMENT.stop) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }

  if(!glitter) return STRIP_MIN_DELAY;

  addSparks(10, false, true);

  return STRIP_MIN_DELAY;

}

/*
 * fade out function
 * fades out the current segment by dividing each pixel's intensity by 2
 */
void WS2812FX::fade_out(uint8_t fadeB = 32) {
  fadeToBlackBy(&leds[SEGMENT.start], SEGMENT_LENGTH, fadeB);
}

/* 
 * <End> Helper Functions
 */


/* 
 * <Begin> User Interface Functions (setables and getables)
 */

/*
 * Lets us set the Blend type (No blend or Linear blend).
 * This affects most effects.
 */
void WS2812FX::setBlendType(TBlendType t = LINEARBLEND) {
  SEGMENT.blendType = t;
}

/*
 * Lets us toggle the Blend type
 */
void WS2812FX::toggleBlendType(void){
  SEGMENT.blendType == NOBLEND ? SEGMENT.blendType = LINEARBLEND : SEGMENT.blendType = NOBLEND;
}

/* 
 * Immediately change the cureent palette to 
 * the one provided - this will not blend to the new palette
 */
void WS2812FX::setCurrentPalette(CRGBPalette16 p, String Name = "Custom") { 
  _currentPalette = p;
  _currentPaletteName = Name;
  _currentPaletteNum = NUM_PALETTES;
}

/* 
 * Immediately change the cureent palette to 
 * the one provided - this will not blend to the new palette
 * n: Number of the Palette to be chosen.
 */
void WS2812FX::setCurrentPalette(uint8_t n=0) { 
  _currentPalette = *(_palettes[n % NUM_PALETTES]);
  _currentPaletteName = _pal_name[n % NUM_PALETTES];
  _currentPaletteNum = n % NUM_PALETTES;
}

/*
 * Set the palette we slowly fade/blend towards.
 * p: the Palette
 * Name: The name
 */
void WS2812FX::setTargetPalette(CRGBPalette16 p, String Name = "Custom") { 
  for(uint8_t i = 0; i< NUM_PALETTES; i++)
  {
    String tName = getPalName(i);
    if(tName == Name)
    {
      setTargetPalette(i);
      return;
    }
  }
  _targetPalette = p;
  _targetPaletteName = Name;
  _targetPaletteNum = NUM_PALETTES;
}

/*
 * Set the palette we slowly fade/blend towards.
 * n: Number of the Palette to be chosen.
 */
void WS2812FX::setTargetPalette(uint8_t n=0) {
  _targetPalette = *(_palettes[n % NUM_PALETTES]);
  _targetPaletteName = _pal_name[n % NUM_PALETTES];
  _targetPaletteNum = n % NUM_PALETTES;
}

/*
 * Change to the mode being provided
 * m: mode number
 */ 
void WS2812FX::setMode(uint8_t m) {
  if(m == SEGMENT.mode) return;  // not really a new mode...
  
  // make sure its a valid mode
  SEGMENT.mode = constrain(m, 0, MODE_COUNT - 1);
  if(!_transition)
  {
    // if we are not currently in a transition phase
    // we clear the led array (the one holding the effect
    // the real LEDs are drawn from _bleds and blended to the leds)
    fill_solid(leds, SEGMENT_LENGTH, CRGB::Black);
  }
  // start the transition phase
  _transition = true;
  _blend = 0;
  
  //setBrightness(_brightness);
}

void WS2812FX::setSpeed(uint16_t s) {
  //This - now actually sets a "beat"
  _segments[0].beat88 = constrain(s, BEAT88_MIN, BEAT88_MAX);
  SEGMENT_RUNTIME.timebase = millis();
}

void WS2812FX::increaseSpeed(uint8_t s) {
  uint16_t newSpeed = constrain(SEGMENT.beat88 + s, BEAT88_MIN, BEAT88_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::decreaseSpeed(uint8_t s) {
  uint16_t newSpeed = constrain(SEGMENT.beat88 - s, BEAT88_MIN, BEAT88_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(CRGBPalette16(((uint32_t)r << 16) | ((uint32_t)g << 8) | b));
}

void WS2812FX::setColor(CRGBPalette16 c) {
  //_segments[0].cPalette = c;
  setTargetPalette(c);  
}

void WS2812FX::setColor(uint32_t c) {
 
  setColor(CRGBPalette16(c));
  setBrightness(_brightness);
}

void WS2812FX::setBrightness(uint8_t b) {
  _brightness = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
 
  FastLED.setBrightness(_brightness);
  FastLED.show();
}

void WS2812FX::increaseBrightness(uint8_t s) {
  s = constrain(_brightness + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::decreaseBrightness(uint8_t s) {
  s = constrain(_brightness - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::setLength(uint16_t b) {
  
}

void WS2812FX::increaseLength(uint16_t s) {
 
}

void WS2812FX::decreaseLength(uint16_t s) {
 
}

boolean WS2812FX::isRunning() {
  return _running;
}

uint8_t WS2812FX::getMode(void) {
  if(_new_mode != 255) {
    return _new_mode;
  }
  else
  {
    return SEGMENT.mode;
  }
}

uint16_t WS2812FX::getBeat88(void) {
  return SEGMENT.beat88;
}

uint8_t WS2812FX::getBrightness(void) {
  return _brightness;
}

uint16_t WS2812FX::getLength(void) {
  return SEGMENT.stop - SEGMENT.start + 1;
}

uint8_t WS2812FX::getModeCount(void) {
  return MODE_COUNT;
}

uint8_t WS2812FX::getPalCount(void) {
  return NUM_PALETTES;
}

uint8_t WS2812FX::getNumSegments(void) {
  return _num_segments;
}

void WS2812FX::setNumSegments(uint8_t n) {
  _num_segments = n;
}

uint32_t WS2812FX::getColor(uint8_t p_index = 0) {
  return ColorFromPalette(_currentPalette, p_index);
}

WS2812FX::segment* WS2812FX::getSegments(void) {
  return _segments;
}

const __FlashStringHelper* WS2812FX::getModeName(uint8_t m) {
  if(m < MODE_COUNT) {
    return _name[m];
  } else {
    return F("");
  }
}

const __FlashStringHelper* WS2812FX::getPalName(uint8_t p) {
  if(p < NUM_PALETTES) {
    return _pal_name[p];
  } else {
    return F("");
  }
}


void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t beat88, bool reverse) {
  if(n < (sizeof(_segments) / sizeof(_segments[0]))) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].beat88 = beat88;
    _segments[n].reverse = reverse;
    //_segments[n].cPalette = CRGBPalette16(color);
  }
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, CRGBPalette16 pal, uint16_t beat88, bool reverse) {
  if(n < (sizeof(_segments) / sizeof(_segments[0]))) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].beat88 = beat88;
    _segments[n].reverse = reverse;
    //_segments[n].cPalette = pal;
  }
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t beat88, bool reverse) {
  if(n < (sizeof(_segments) / sizeof(_segments[0]))) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].beat88 = beat88;
    _segments[n].reverse = reverse;
    /*
    for(uint8_t i=0; i<NUM_COLORS; i++) {
      _segments[n].cPalette = CRGBPalette16(colors[i]);
    }
    */
  }
}

void WS2812FX::resetSegments() {
  memset(_segments, 0, sizeof(_segments));
  memset(_segment_runtimes, 0, sizeof(_segment_runtimes));
  _segment_index = 0;
  _num_segments = 1;
  setSegment(0, 0, 7, FX_MODE_STATIC, DEFAULT_COLOR, DEFAULT_BEAT88, false);
}

uint16_t (*customMode)(void) = NULL;

/*
 * Custom mode helper
 */
void WS2812FX::setCustomMode(uint16_t (*p)()) {
  setMode(FX_MODE_CUSTOM);
  customMode = p;
}

/* 
 * <End> User Interface Functions (setables and getables)
 */


/* #####################################################
#
#  Color and Blinken Functions
#
##################################################### */


/*
 * The "Off" mode clears the Leds.
 *
uint16_t WS2812FX::mode_off(void) {
  FastLED.clear(true);
  return 1000;
}
*/

/*
 * No blinking. Just plain old static light - but mapped on a color palette.
 * Palette ca be "moved" by SEGMENT.baseHue
 * will distribute the palette over the display length
 */
uint16_t WS2812FX::mode_static(void) {
  
  fill_palette(&leds[SEGMENT.start], SEGMENT_LENGTH, SEGMENT_RUNTIME.baseHue, (SEGMENT_LENGTH > 255 ? 1 : (255 / SEGMENT_LENGTH) + 1), _currentPalette, _brightness, SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 */
uint16_t WS2812FX::mode_ease(void) {
  return this->mode_ease_func(false);
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 * Random Sparkles will be additionally applied.
 */
uint16_t WS2812FX::mode_twinkle_ease(void) {
  return this->mode_ease_func(true);
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 * Random Sparkles can additionally applied.
 */
uint16_t WS2812FX::mode_ease_func(bool sparks = true) {
  // number of pixels for "antialised" (fractional) bar
  const uint8_t width = 1;
  // pixel position on the strip we make two out of it...
  uint16_t lerpVal    = 0;
  // need to know if we are in the middle (to smoothly update random beat)
  static bool trigger = false;
  // beat being modified during runtime
  static uint16_t beat = SEGMENT.beat88;
  // to check if beat88 recently changed
  // ToDo (idea) maybe a global runtime flag could help 
  // which is recent by the active effect making use of the "beat"
  static uint16_t oldbeat = SEGMENT.beat88;
  // to check if we have movement.
  // maybe easier but works good for now.
  static uint16_t p_lerp = lerpVal;

  // instead of moving the color around (palette wise)
  // we set it to the baseHue. So it can still be changed 
  // and also change over time
  uint8_t colorMove = SEGMENT_RUNTIME.baseHue;  //= quadwave8(map(beat88(max(SEGMENT.beat88/2,1),SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)) + SEGMENT_RUNTIME.baseHue;

  // this is the fading tail....
  // we adjust it a bit on the speed (beat)
  fade_out(SEGMENT.beat88 >> 5);

  // now e calculate a sine curve for the led position
  // factor 16 is used for the fractional bar
  lerpVal = beatsin88(beat, SEGMENT.start*16, SEGMENT.stop*16-(width*16), SEGMENT_RUNTIME.timebase);
  
  // once we are in the middle
  // we can modify the speed a bit
  if(lerpVal == ((SEGMENT_LENGTH*16)/2))
  {
    // the trigger is used because we are more frames in the middle 
    // but only one should trigger
    if(trigger)
    {
      // if the changed the base speed (external source)
      // we refesh the values
      if(oldbeat != SEGMENT.beat88)
      {
        beat = SEGMENT.beat88;
        oldbeat = SEGMENT.beat88;
        //SEGMENT_RUNTIME.timebase = millis();
      }
      // reset the trigger
      trigger = false;
      // tiimebase starts fresh in the middle (avoid jumping pixels)
      SEGMENT_RUNTIME.timebase = millis();
      // we randomly increase or decrease
      // as we work with unsigned values we do this with an offset...
      // smallest value should be 255
      if(beat < 255)
      {
        // avoid roll over to 65535
        beat += 2 * random8();
      }
      else
      {
        // randomly increase or decrease beat
        beat += 2 * (128 - random8());
      }
      
    }
  }
  else
  {
    // activate trigger if we are moving
    if(lerpVal != p_lerp) trigger = true;
  }

  p_lerp = lerpVal;
  // we draw two fractional bars here. for the color mapping we need the overflow and therefore cast to uint8_t
  drawFractionalBar(lerpVal, width, _currentPalette, (uint8_t)((uint8_t)(lerpVal/16-SEGMENT.start) + colorMove), _brightness);
  drawFractionalBar((SEGMENT.stop*16)-lerpVal, width, _currentPalette, (uint8_t)((uint8_t)(lerpVal/16-SEGMENT.start) + colorMove), _brightness);

  if(sparks) addSparks(10, true, false);

  return STRIP_MIN_DELAY;
}

// moves a fractional bar along the stip based on noise
uint16_t WS2812FX::mode_inoise8_mover(void) {
  return this->mode_inoise8_mover_func(false);
}

// moves a fractional bar along the stip based on noise
// random twinkles are added
uint16_t WS2812FX::mode_inoise8_mover_twinkle(void) {
  return this->mode_inoise8_mover_func(true);
}

uint16_t WS2812FX::mode_inoise8_mover_func(bool sparks) {
  uint16_t xscale = SEGMENT_LENGTH; //30;                                         
  uint16_t yscale = 30;
  const uint16_t width = 6; //max(SEGMENT.beat88/256,1);
  static uint16_t dist = 1234;
  
  uint8_t locn = inoise8(xscale, dist+yscale);       
  uint16_t pixlen = map(locn,0,255,SEGMENT.start*16, SEGMENT.stop*16-width*16);
  
  uint8_t colormove = SEGMENT_RUNTIME.baseHue; // quadwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)) + SEGMENT_RUNTIME.baseHue;

  fade_out(48);
  
  drawFractionalBar(pixlen, width, _currentPalette, (uint8_t)((uint8_t)(pixlen / 64) + colormove)); //, beatsin88(max(SEGMENT.beat88/2,1),200 % _brightness, _brightness, SEGMENT_RUNTIME.timebase));
  
  dist += beatsin88(SEGMENT.beat88,1,6, SEGMENT_RUNTIME.timebase);  

  if(sparks) addSparks(10, true, false);

  return STRIP_MIN_DELAY;
}

/*
 * Plasma like Effect over the complete strip.
 */
uint16_t WS2812FX::mode_plasma(void) {
  uint8_t thisPhase = beatsin88(SEGMENT.beat88, 0, 255, SEGMENT_RUNTIME.timebase);                           // Setting phase change for a couple of waves.
  uint8_t thatPhase = beatsin88((SEGMENT.beat88*11)/10, 0, 255, SEGMENT_RUNTIME.timebase); // was int thatPhase = 64 - beatsin88((SEGMENT.beat88*11)/10, 0, 128, SEGMENT_RUNTIME.timebase);

  for (int k=SEGMENT.start; k<SEGMENT.stop; k++) {                              // For each of the LED's in the strand, set a brightness based on a wave as follows:

    uint8_t colorIndex = cubicwave8((k*15)+thisPhase)/2 + cos8((k*8)+thatPhase)/2 + SEGMENT_RUNTIME.baseHue;           // Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
    uint8_t thisBright = qsuba(colorIndex, beatsin88((SEGMENT.beat88*12)/10,0,128));             // qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..
    CRGB newColor = ColorFromPalette(_currentPalette, colorIndex, thisBright, SEGMENT.blendType);  // Let's now add the foreground colour.
    leds[k] = nblend(leds[k], newColor, 64);
  }
  return STRIP_MIN_DELAY;
}

/*
 * Move 3 dots / small bars (antialised) at different speeds
 */
uint16_t WS2812FX::mode_juggle_pal(void) {
  const uint8_t numdots = 3;
  const uint8_t width = max(SEGMENT_LENGTH/15,2);
  uint8_t curhue = 0;
  static uint8_t thishue = 0;
  curhue = thishue;                                           // Reset the hue values.
  EVERY_N_MILLISECONDS(100)
  {
    thishue = random8(curhue, qadd8(curhue,8));
  }

  fade_out(96);
  
  for( int i = 0; i < numdots; i++) {
    uint16_t pos = beatsin88(max(SEGMENT.beat88/2,1)+i*256+762,SEGMENT.start*16, SEGMENT.stop*16-width*16, SEGMENT_RUNTIME.timebase);
    drawFractionalBar(pos, width, _currentPalette, curhue, _brightness);
    uint8_t delta = random8(9);
    if(delta < 5)
    {
      curhue = curhue - (uint8_t)(delta)  + SEGMENT_RUNTIME.baseHue;
    }
    else
    {
      curhue = curhue + (uint8_t)(delta/2) + SEGMENT_RUNTIME.baseHue;
    }
    
  }
  return STRIP_MIN_DELAY;
}

/*
 * Confetti, yeah
 */
uint16_t WS2812FX::mode_confetti(void) {

  fade_out(8);
  
  if(random8(3) != 0) return 20;

  uint16_t pos;
  uint8_t index = (uint8_t)beatsin88(SEGMENT.beat88, 0, 255, SEGMENT_RUNTIME.timebase) + SEGMENT_RUNTIME.baseHue;
  uint8_t bright = random8(192 % _brightness, _brightness);
  const uint8_t space = 1;
  bool newSpark = true;
  
  pos = random16((SEGMENT.start + 1)*16, (SEGMENT.stop - 2)*16-32);   
  for(int_fast8_t i = 0 - space; i<=space; i++)
  {
    if((pos/16+i) >=SEGMENT.start && (pos/16+i) < SEGMENT.stop)
    {
      if(leds[(pos/16+i)]) newSpark = false;
    }
  }

  if(!newSpark) return STRIP_MIN_DELAY;

  drawFractionalBar(pos, 1, _currentPalette, index, bright);
  
  return STRIP_MIN_DELAY;
}


#pragma message "Needs Rework, especially beat89 and effect width - currently not used..."
/*
 * Fills the strip with waving color and brightness
 */
uint16_t WS2812FX::mode_fill_beat(void) {
  
  uint8_t dist1, dist2;
  dist1 = (uint8_t)((triwave8(map(SEGMENT.beat88*6, 0, 65535, 0, 255)) + (uint8_t)beatsin88(SEGMENT.beat88, 0, 5, SEGMENT_RUNTIME.timebase)));
  dist2 = (uint8_t)((uint8_t)map(  beat88( max((SEGMENT.beat88*3)*2,1), 
                        SEGMENT_RUNTIME.timebase),
                0 , 65535, 0, 255) 
          + 
          (uint8_t)beatsin88(  max(SEGMENT.beat88, (uint16_t)1), 0, 4, SEGMENT_RUNTIME.timebase));
  //dist1 = map8(dist1,0,255);
  //dist2 = map8(dist2,0,255);
  CRGB newColor = CRGB::Black;
  for(uint8_t k=SEGMENT.start; k<SEGMENT.stop; k++)
  {
    uint8_t br = quadwave8(k*2-dist1) % _brightness;
    newColor = ColorFromPalette(_currentPalette, k+dist2 + SEGMENT_RUNTIME.baseHue, br, SEGMENT.blendType); 
    leds[k] = nblend(leds[k], newColor, qadd8(SEGMENT.beat88>>8, 24));
  }
  return STRIP_MIN_DELAY;
}

/*
 * Wave Effect over the complete strip.
 */
uint16_t WS2812FX::mode_fill_wave(void) {
  fill_palette( &leds[SEGMENT.start], 
                (SEGMENT_LENGTH), 
                SEGMENT_RUNTIME.baseHue + (uint8_t)beatsin88(SEGMENT.beat88*2, 0, 255, SEGMENT_RUNTIME.timebase),
                // SEGMENT_RUNTIME.baseHue + triwave8( (uint8_t)map( beat88( max(  SEGMENT.beat88/4, 2), SEGMENT_RUNTIME.timebase), 0,  65535,  0,  255)),
                          max(  255/SEGMENT_LENGTH+1, 1), 
                _currentPalette, 
                (uint8_t)beatsin88(  max(SEGMENT.beat88*1 , 1), 
                            48, 255, 
                            SEGMENT_RUNTIME.timebase),
                SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}


/*
 * 3 "dots / small bars" moving with different 
 * wave functions and different speed.
 */
uint16_t WS2812FX::mode_dot_beat(void) {
  static uint16_t beats[] = { 
     max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88), 
     max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88), 
     max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88)
  };
  
  static uint16_t oldB = SEGMENT.beat88;
  if(oldB != SEGMENT.beat88)
  {
    oldB = SEGMENT.beat88;
    beats[0] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88); 
    beats[1] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88); 
    beats[2] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88);
  }

  #pragma message "Use Runtime Parameters for these Variables.... Maybe a union can help."
  static uint32_t timebase[] = {millis(), millis(), millis()};
  static bool newbase[] = {false, false, false};
  uint16_t cled = 0;
  const uint8_t width = 2;//max(SEGMENT_LENGTH/15, 2);
  
  static uint8_t coff[] = { 
                            random8(SEGMENT_RUNTIME.baseHue, 32 + SEGMENT_RUNTIME.baseHue), 
                            random8(SEGMENT_RUNTIME.baseHue, 32 + SEGMENT_RUNTIME.baseHue), 
                            random8(SEGMENT_RUNTIME.baseHue, 32 + SEGMENT_RUNTIME.baseHue)
                          };

  fade_out(64);


  for(uint8_t i=0; i< 3; i++)
  {
    uint8_t cind = 0;
    switch (i)
    {
      case 0:
        cled = map(triwave16(beat88(beats[i], timebase[i])), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
        
      break;
      case 1:
        cled = map(quadwave16(beat88(beats[i], timebase[i])), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
        
      break;
      case 2:
        cled = map(cubicwave16(beat88(beats[i], timebase[i])), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
        
      break;
      default:
        cled = map(quadwave16(beat88(beats[i], timebase[i])), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
        
      break;
    }

    if(cled == SEGMENT.start*16)
    {
      if(newbase[i]) 
      {
        timebase[i] = millis();
        newbase[i] = false;
      }
      beats[i] = max((uint16_t)(beats[i] + (int16_t)((int16_t)256 - (int16_t)random16(0 , 512))), SEGMENT.beat88);
      


      if(beats[i] <= 256) beats[i] = 256;
      if(beats[i] >= 65535-512) beats[i] = 65535-512;
      
      coff[i] = random8(SEGMENT_RUNTIME.baseHue, 64 + SEGMENT_RUNTIME.baseHue);
    }
    else
    {
      newbase[i] = true;
    }

    cind = coff[i] + map(cled/16, SEGMENT.start, SEGMENT.stop , 0, 255);

    drawFractionalBar(cled, width, _currentPalette, cind, _brightness);
   
  }
  return STRIP_MIN_DELAY;
}


/*
 * Pulsing to the inner middle from both ends..
 */
uint16_t WS2812FX::mode_to_inner(void) {
  #pragma message "Implement fractional grow"
  
  uint16_t led_up_to = (((SEGMENT_LENGTH)/2+1)+SEGMENT.start);

  //fadeToBlackBy(&leds[SEGMENT.start], (SEGMENT_LENGTH), 64);
  fade_out(64);
  
  //start = beatsin16(beat/3, SEGMENT.start, led_up_to/2);
  fill_palette(&leds[SEGMENT.start], 
               beatsin88(
                         SEGMENT.beat88 < 13107 ? SEGMENT.beat88 * 5 : SEGMENT.beat88, 
                         0, led_up_to, SEGMENT_RUNTIME.timebase), 
               SEGMENT_RUNTIME.baseHue, 5, _currentPalette, 255, SEGMENT.blendType);
  for(uint8_t i = (SEGMENT_LENGTH)-1; i>=((SEGMENT_LENGTH) - led_up_to); i--)
  {
    if(((SEGMENT_LENGTH)-i) >= 0 && ((SEGMENT_LENGTH)-i) < (SEGMENT_LENGTH))
    {
      leds[i] = leds[(SEGMENT_LENGTH)-i];
    }
  }
  return STRIP_MIN_DELAY;
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void) {
  
  fill_palette(&leds[SEGMENT.start], SEGMENT_LENGTH, 0 + SEGMENT_RUNTIME.baseHue, 5, _currentPalette, beatsin88(SEGMENT.beat88 * 2, 15, 255, SEGMENT_RUNTIME.timebase), SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_multi_dynamic(void) {
  #pragma message "Repair me!! too fast and no idea why..."
  static uint32_t last = 0;
  if(millis() > last)
  {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
        
      leds[i] = ColorFromPalette(_currentPalette, get_random_wheel_index(i>SEGMENT.start?i-1:i), _brightness, SEGMENT.blendType);
        
    }
    last = millis() + ((BEAT88_MAX - SEGMENT.beat88)>>7);
  }

  return STRIP_MIN_DELAY; //(BEAT88_MAX - SEGMENT.beat88) / 256;
}


/*
 * Waving brightness over the complete strip.
 */
uint16_t WS2812FX::mode_fill_bright(void) {
  fill_palette(&leds[SEGMENT.start], (SEGMENT_LENGTH), beat88(max((SEGMENT.beat88/56),2), SEGMENT_RUNTIME.timebase), 
      max(255/SEGMENT_LENGTH+1,1), _currentPalette, beatsin88(max(SEGMENT.beat88/112,1), 16, 255, SEGMENT_RUNTIME.timebase),SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}


uint16_t WS2812FX::mode_firework(void){
  const uint8_t dist = 1;
  //fade_out(qadd8(SEGMENT.beat88 >> 8, 2));
  blur1d(&leds[SEGMENT.start], SEGMENT_LENGTH, qadd8(255-(SEGMENT.beat88 >> 8), 2)%172);
  if(random8(max(6, SEGMENT_LENGTH/7)) <= max(3, SEGMENT_LENGTH/14)) 
  {
    uint8_t lind = random16(dist+SEGMENT.start, SEGMENT.stop-dist);
    uint8_t cind = random8() + SEGMENT_RUNTIME.baseHue;
    for(int8_t i = 0-dist; i<=dist; i++)
    {
      if(lind+i >= SEGMENT.start && lind + i < SEGMENT.stop)
      {
        if(!(leds[lind+i] == CRGB(0x0))) return STRIP_MIN_DELAY;
      }
    }
    leds[lind] = ColorFromPalette(_currentPalette, cind  , 255, SEGMENT.blendType);
  }
  return STRIP_MIN_DELAY; // (BEAT88_MAX - SEGMENT.beat88) / 256; // STRIP_MIN_DELAY;
}

/*
 * Fades the LEDs on and (almost) off again.
 */
uint16_t WS2812FX::mode_fade(void) {
  //int lum = SEGMENT_RUNTIME.counter_mode_step - 31;
  //lum = 63 - (abs(lum) * 2);
  //lum = map(lum, 0, 64, 25, 255);

  fill_palette(&(leds[SEGMENT.start]), SEGMENT_LENGTH, 0 + SEGMENT_RUNTIME.baseHue, 5, _currentPalette, map8(triwave8(map(beat88(SEGMENT.beat88*10, SEGMENT_RUNTIME.timebase),0,65535,0,255)),24,255), SEGMENT.blendType);
  
  //SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 2) % 64;
  return STRIP_MIN_DELAY; //(SEGMENT.speed / 64);
}

/*
 * Runs a single pixel back and forth.
 */
uint16_t WS2812FX::mode_scan(void) {
  //uint16_t led_offset = map(triwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)), 0, 255, SEGMENT.start*16, SEGMENT.stop*16);
  const uint16_t width = 2; // max(2, SEGMENT_LENGTH/50)
  uint16_t led_offset = map(triwave16(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase)), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
  
  // maybe we change this to fade?
  fill_solid(&(leds[SEGMENT.start]), SEGMENT_LENGTH, CRGB(0, 0, 0));

  if(SEGMENT.reverse) {
    drawFractionalBar(SEGMENT.stop  * 16 - led_offset, width, _currentPalette, 255-led_offset/16 + SEGMENT_RUNTIME.baseHue, _brightness);
  } else {
    drawFractionalBar(SEGMENT.start * 16 + led_offset, width, _currentPalette, led_offset/16 + SEGMENT_RUNTIME.baseHue, _brightness);
  }

  return STRIP_MIN_DELAY;
  
}

/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void) {
  //uint16_t led_offset = map(triwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)), 0, 255, SEGMENT.start*16, SEGMENT.stop*16);
  const uint16_t width = 2; // max(2, SEGMENT_LENGTH/50)
  uint16_t led_offset = map(triwave16(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase)), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16- width*16);
  

  fill_solid(&(leds[SEGMENT.start]), SEGMENT_LENGTH, CRGB(0, 0, 0));

  drawFractionalBar(SEGMENT.stop  * 16 - led_offset, width, _currentPalette, 255-led_offset/16 + SEGMENT_RUNTIME.baseHue, _brightness); 
  drawFractionalBar(SEGMENT.start * 16 + led_offset, width, _currentPalette, led_offset/16 + SEGMENT_RUNTIME.baseHue, _brightness);

  return STRIP_MIN_DELAY;
}

/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void) {
  
  fill_solid(&leds[SEGMENT.start], SEGMENT_LENGTH, ColorFromPalette(_currentPalette, map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255),_brightness, SEGMENT.blendType)); /*CHSV(beat8(max(SEGMENT.beat/2,1), SEGMENT_RUNTIME.timebase)*/ //_brightness));
  //SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 2) & 0xFF;
  return STRIP_MIN_DELAY; 
}

/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void) {
 
  fill_palette( &leds[SEGMENT.start], 
                SEGMENT_LENGTH, 
                map(  beat88( SEGMENT.beat88, 
                              SEGMENT_RUNTIME.timebase), 
                      0, 65535, 0, 255), 
                (SEGMENT_LENGTH > 255 ? 1 : (255/SEGMENT_LENGTH)+1), 
                _currentPalette, 
                255, SEGMENT.blendType);
  
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_pride(void) {
  return pride(false);
}

uint16_t WS2812FX::mode_pride_glitter(void) {
  return pride(true);
}

/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(CRGBPalette16 color1, CRGBPalette16 color2) {
  uint16_t off = map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255) % 3;
  
  for(uint16_t i=0; i<SEGMENT_LENGTH; i++)
  {
    uint8_t pal_index = map(i, 0, SEGMENT_LENGTH-1, 0, 255) + SEGMENT_RUNTIME.baseHue;
    if((i % 3) == off) {  
      if(SEGMENT.reverse) {
        leds[SEGMENT.stop - i] = ColorFromPalette(color1, pal_index, _brightness, SEGMENT.blendType);
      } else {
        leds[SEGMENT.start + i] = ColorFromPalette(color1, pal_index, _brightness, SEGMENT.blendType);
      }
    } else {
      if(SEGMENT.reverse) {
        leds[SEGMENT.stop - i] = ColorFromPalette(color2, 128 + pal_index, _brightness, SEGMENT.blendType);
      } else {
        leds[SEGMENT.start + i] = ColorFromPalette(color2, 128 + pal_index, _brightness, SEGMENT.blendType);
      }
    }
  }
  return STRIP_MIN_DELAY;
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void) {
  return theater_chase(_currentPalette, CRGBPalette16(CRGB::Black));
}

uint16_t WS2812FX::mode_theater_chase_dual_pal(void) {
  return theater_chase(_currentPalette, _currentPalette);
}

/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void) {
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return theater_chase(CRGBPalette16(ColorFromPalette(_currentPalette, SEGMENT_RUNTIME.counter_mode_step)), CRGBPalette16(CRGB::Black));
}

/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void) {
 
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    uint8_t lum = qsub8(sin8_C(map(i,0, SEGMENT_LENGTH-1, 0, 255)), 2);
    uint16_t offset = map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, SEGMENT_LENGTH-1);
    offset = (offset+i)%SEGMENT_LENGTH;

    CRGB newColor = CRGB::Black;
    
    if(SEGMENT.reverse) {
      newColor = ColorFromPalette(_currentPalette, map(SEGMENT.stop - offset, SEGMENT_LENGTH-1, 0, 0, 255) + SEGMENT_RUNTIME.baseHue, lum, SEGMENT.blendType);
      nblend (leds[SEGMENT.stop - offset], newColor, 64);
    } else {
      newColor = ColorFromPalette(_currentPalette, map(offset, 0, SEGMENT_LENGTH-1, 0, 255) + SEGMENT_RUNTIME.baseHue, lum, SEGMENT.blendType); 
      nblend (leds[SEGMENT.start + offset], newColor, qadd8(SEGMENT.beat88>>8, 16));
    }
  }
  
  return STRIP_MIN_DELAY;
}

/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade(void) {
  fade_out(qadd8(SEGMENT.beat88>>8, 12));
  addSparks(4, true, false);
  return STRIP_MIN_DELAY;
}

/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void) {
  
  const uint16_t width = max(1,SEGMENT_LENGTH/15);
  fade_out(96);

  uint16_t pos = triwave16(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase));
  //map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, SEGMENT_LENGTH*2*16-width*16);
  pos = map(pos, 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);

  /*
  if(pos < SEGMENT_LENGTH*16) {
    if(SEGMENT.reverse) {
      drawFractionalBar(SEGMENT.stop*16 - pos, 
                        width, 
                        _currentPalette, 
                        SEGMENT_RUNTIME.baseHue + map(SEGMENT.stop*16 - pos, SEGMENT.start*16, 
                        SEGMENT.stop*16, 0, 255)); 
    } else {
      drawFractionalBar(SEGMENT.start*16 + pos, 
                        width, 
                        _currentPalette, 
                         SEGMENT_RUNTIME.baseHue + map(SEGMENT.start*16 + pos, SEGMENT.start*16, 
                        SEGMENT.stop*16, 0, 255));
    }
  } else {
    if(SEGMENT.reverse) {
      drawFractionalBar(SEGMENT.stop*16 - ((SEGMENT_LENGTH * 2 * 16) - pos) + 2, 
                        width, 
                        _currentPalette, 
                        SEGMENT_RUNTIME.baseHue + map(SEGMENT.stop*16 - ((SEGMENT_LENGTH * 2 * 16) - pos) + 2, 
                        SEGMENT.start*16, SEGMENT.stop*16, 0, 255));

    } else {
       drawFractionalBar(SEGMENT.start*16 + ((SEGMENT_LENGTH * 2 * 16) - pos) - 2, 
                         width, 
                         _currentPalette, 
                         SEGMENT_RUNTIME.baseHue + map(SEGMENT.start*16 + ((SEGMENT_LENGTH * 2 * 16) - pos) - 2, 
                         SEGMENT.start*16, SEGMENT.stop*16, 0, 255));
    }
  }
  */
  drawFractionalBar(pos, 
                    width, 
                    _currentPalette, 
                    SEGMENT_RUNTIME.baseHue + map(pos, SEGMENT.start*16, SEGMENT.stop*16-width*16, 0, 255));

  //SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + ((uint8_t)map(SEGMENT.beat, BEAT88_MIN, BEAT88_MAX, 96, 1))) % (((SEGMENT_LENGTH * 2) - 2)*16);
  return STRIP_MIN_DELAY;//(255-SEGMENT.beat) * 3 / SEGMENT_LENGTH; //return (SEGMENT.speed / ((SEGMENT_LENGTH * 8)));  
}

/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void) {
  const uint16_t width = max(1,SEGMENT_LENGTH/15);
  fade_out(96);

  uint16_t pos = map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, SEGMENT_LENGTH*16);

  if(SEGMENT.reverse) {
    drawFractionalBar((SEGMENT.stop*16 - pos), 
                        width, 
                        _currentPalette, 
                        map(SEGMENT.stop*16 - pos, SEGMENT.start*16, SEGMENT.stop*16, 0, 255) + SEGMENT_RUNTIME.baseHue);
  } else {
    drawFractionalBar((SEGMENT.start*16 + pos), 
                        width, 
                        _currentPalette, 
                        map(SEGMENT.start*16 + pos, SEGMENT.start*16, SEGMENT.stop*16, 0, 255) + SEGMENT_RUNTIME.baseHue);
  }
  //SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + ((uint8_t)map(SEGMENT.beat, BEAT88_MIN, BEAT88_MAX, 96, 1))) % (SEGMENT_LENGTH*16);
  return STRIP_MIN_DELAY; //(255-SEGMENT.beat) * 3 / SEGMENT_LENGTH; // return (SEGMENT.speed / (SEGMENT_LENGTH*4));
}

/*
 * Fire flicker function
 */
uint16_t WS2812FX::fire_flicker(int rev_intensity) {
 
  byte lum = 255 / rev_intensity;
  
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    int flicker = random8(0, lum);
    leds[i] = ColorFromPalette(_currentPalette, map(i, SEGMENT.start, SEGMENT.stop, 0, 255) + SEGMENT_RUNTIME.baseHue, _brightness, SEGMENT.blendType);   
    leds[i] -= CRGB(random8(flicker), random8(flicker), random8(flicker));
  }
  return (BEAT88_MAX - SEGMENT.beat88) / 256; // SEGMENT.beat * 100 / SEGMENT_LENGTH; //return (SEGMENT.speed / SEGMENT_LENGTH);
}

/*
 * Random flickering.
 */
uint16_t WS2812FX::mode_fire_flicker(void) {
  return fire_flicker(4);
}

/*
 * Random flickering, less intesity.
 */
uint16_t WS2812FX::mode_fire_flicker_soft(void) {
  return fire_flicker(6);
}

/*
 * Random flickering, more intesity.
 */
uint16_t WS2812FX::mode_fire_flicker_intense(void) {
  return fire_flicker(2);
}

uint16_t WS2812FX::mode_bubble_sort(void) {
  static uint8_t *hues;
  static bool init = true;
  static bool movedown = false;
  static uint16_t ci = 0;
  static uint16_t co = 0;
  static uint16_t cd = 0;
  if(init) {
    init = false;
    hues = new uint8_t[SEGMENT_LENGTH];
    for(uint8_t i=0; i<SEGMENT_LENGTH; i++)
    {
      hues[i] = random8();
    }
    map_pixels_palette(hues, 32, SEGMENT.blendType);
    co = 0;
    ci = 0;
    return STRIP_MIN_DELAY;
  }
  if(!movedown) {
    if(co <= SEGMENT_LENGTH) {
      if(ci <= SEGMENT_LENGTH)
      {
        if(hues[ci] > hues[co])
        {
          uint8_t tmp = hues[ci];
          hues[ci] = hues[co];
          hues[co] = tmp;
          cd = ci;
          movedown = true;
        }
        ci++;
      }
      else
      {
        co++;
        ci = co;
      }
    }
    else
    {
      delete hues;
      init = true;
      return 5000;
    }
    map_pixels_palette(hues, 32, SEGMENT.blendType);
    leds[ci + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[ci], _brightness, SEGMENT.blendType); //CRGB::Green;
    leds[co + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[ci], _brightness, SEGMENT.blendType); // CRGB::Red;
  }
  else
  {
    map_pixels_palette(hues, 32, SEGMENT.blendType);
    leds[co + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[ci], _brightness, SEGMENT.blendType); // CRGB::Red;
    leds[cd + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[cd], _brightness, SEGMENT.blendType); // +=CRGB::Green;
    if(cd == co) {
      movedown = false;
    }
    cd --;
    return STRIP_MIN_DELAY;
  }
  return STRIP_MIN_DELAY;
}

/* 
 * Fire with Palette
 */
uint16_t WS2812FX::mode_fire2012WithPalette(void) {
  //uint8_t cooling = map(SEGMENT.beat88, BEAT88_MIN, BEAT88_MAX, 20, 100);
  //uint8_t sparking = map(SEGMENT.beat88, BEAT88_MIN, BEAT88_MAX, 50, 200);
  // Array of temperature readings at each simulation cell
  static byte *heat = new byte[SEGMENT_LENGTH];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < SEGMENT_LENGTH; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((_segments[0].cooling * 10) / SEGMENT_LENGTH) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= SEGMENT_LENGTH - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < _segments[0].sparking ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < SEGMENT_LENGTH; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( _currentPalette, colorindex);
      int pixelnumber;
      if( SEGMENT.reverse ) {
        pixelnumber = (SEGMENT_LENGTH-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber + SEGMENT.start] = color;
    }
    return STRIP_MIN_DELAY;
}


/*
 * TwinleFox Implementation
 */
uint16_t WS2812FX::mode_twinkle_fox( void) {
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;
  
  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  if((_currentPalette[0] == _currentPalette[1] )) {
    bg = _currentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = CRGB::Black; //gBackgroundColor; // just use the explicitly defined background color
  }

  uint8_t backgroundBrightness = bg.getAverageLight();
  
  for( uint16_t i=0; i<SEGMENT_LENGTH; i++) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if( deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color, 
      // use the new color.
      leds[i+SEGMENT.start] = c;
    } else if( deltabright > 0 ) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      leds[+SEGMENT.start] = blend( bg, c, deltabright * 8);
    } else { 
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      leds[i+SEGMENT.start] = bg;
    }
  }
  return STRIP_MIN_DELAY;
}


/*
 * SoftTwinkles
 */
uint16_t WS2812FX::mode_softtwinkles(void) {
  CRGB lightcolor = CRGB (8,7,1);

  for( int i = 0; i < SEGMENT_LENGTH; i++) {
    if( !leds[i+SEGMENT.start]) continue; // skip black pixels
    if( leds[i+SEGMENT.start].r & 1) { // is red odd?
      leds[i+SEGMENT.start] -= lightcolor; // darken if red is odd
    } else {
      leds[i+SEGMENT.start] += lightcolor; // brighten if red is even
    }
  }
  
  // Randomly choose a pixel, and if it's black, 'bump' it up a little.
  // Since it will now have an EVEN red component, it will start getting
  // brighter over time.
  if( random8() < 200 && !_transition) {
    int j = random16(SEGMENT.start+1, SEGMENT.stop-1);
    if( !leds[j] && !leds[j+1] && !leds[j-1]) 
    {
      leds[j] = lightcolor;
    }
  }
  return STRIP_MIN_DELAY;
} 

/*
 * Custom mode
 */


uint16_t WS2812FX::mode_custom() {
  if(customMode == NULL) {
    return mode_static(); // if custom mode not set, we just do "static"
  } else {
    return customMode();
  }
}