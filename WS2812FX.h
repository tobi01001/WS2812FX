/*
  WS2812FX.h - Library for WS2812 LED effects.

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
*/

#ifndef WS2812FX_h
#define WS2812FX_h

/* <FastLED implementation> */
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_DMA
#define FASTLED_USE_PROGMEM 1

#ifndef LED_PIN
  #define LED_PIN 3
#endif

#define STRIP_MIN_DELAY (1000/(STRIP_MAX_FPS))  
#define STRIP_MAX_FPS _fps


#include "FastLED.h"


/* </FastLED implementation> */

#define DEFAULT_BRIGHTNESS 255
#define DEFAULT_MODE 1
#define DEFAULT_BEAT88 255
#define DEFAULT_COLOR 0xFF0000
#define DEFAULT_DELTAHUE 1
#define DEFAULT_HUETIME 500;

#ifdef SPEED_MAX
  #error "SPEED_MAX define is no longer used!"
#endif

#ifdef SPEED_MIN
  #error "SPEED_MIN define is no longer used!"
#endif

#define BEAT88_MIN 1
#define BEAT88_MAX 65535

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 255

/* each segment uses 34 bytes of SRAM memory, so if you're application fails because of
  insufficient memory, decreasing MAX_NUM_SEGMENTS may help */
#define MAX_NUM_SEGMENTS 10
#define NUM_COLORS 1     /* number of colors per segment */
#define SEGMENT          _segments[_segment_index]
#define SEGMENT_RUNTIME  _segment_runtimes[_segment_index]
#define SEGMENT_LENGTH   (SEGMENT.stop - SEGMENT.start + 1)
// ToDo: Reset Runtime as inline funtion with new timebase?"
#define RESET_RUNTIME    memset(_segment_runtimes, 0, sizeof(_segment_runtimes))




// some common colors
#define RED        0xFF0000
#define GREEN      0x00FF00
#define BLUE       0x0000FF
#define WHITE      0xFFFFFF
#define BLACK      0x000000
#define YELLOW     0xFFFF00
#define CYAN       0x00FFFF
#define MAGENTA    0xFF00FF
#define PURPLE     0x400080
#define ORANGE     0xFF3000
#define ULTRAWHITE 0xFFFFFFFF


enum MODES {
  FX_MODE_OFF,
  FX_MODE_STATIC,
  FX_MODE_EASE,
  FX_MODE_TWINKLE_EASE,
  FX_MODE_NOISEMOVER,
  FX_MODE_TWINKLE_NOISEMOVER,
  FX_MODE_PLASMA,
  FX_MODE_JUGGLE_PAL,
  FX_MODE_CONFETTI,
  FX_MODE_FILL_BEAT ,
  FX_MODE_FILL_WAVE ,
  FX_MODE_DOT_BEAT,
  FX_MODE_TO_INNER,
  FX_MODE_BREATH,
  FX_MODE_MULTI_DYNAMIC ,
  FX_MODE_RAINBOW ,
  FX_MODE_RAINBOW_CYCLE ,
  FX_MODE_PRIDE,
  FX_MODE_PRIDE_GLITTER,
  FX_MODE_SCAN,
  FX_MODE_DUAL_SCAN ,
  FX_MODE_FADE,
  FX_MODE_THEATER_CHASE ,
  FX_MODE_THEATER_CHASE_DUAL_P,
  FX_MODE_THEATER_CHASE_RAINBOW ,
  FX_MODE_RUNNING_LIGHTS,
  FX_MODE_TWINKLE_FADE ,
  FX_MODE_TWINKLE_FOX ,
  FX_MODE_SOFTTWINKLES,
  FX_MODE_FILL_BRIGHT ,
  FX_MODE_FIREWORK,
  FX_MODE_FIRE2012,
  FX_MODE_LARSON_SCANNER,
  FX_MODE_COMET ,
  FX_MODE_FIRE_FLICKER,
  FX_MODE_FIRE_FLICKER_SOFT ,
  FX_MODE_FIRE_FLICKER_INTENSE,
  FX_MODE_BUBBLE_SORT,
  FX_MODE_CUSTOM,

  // has to be the final entry!
  MODE_COUNT
};


extern const TProgmemRGBPalette16 
      Ice_Colors_p,
      Ice_p, 
      RetroC9_p, 
      Snow_p, 
      FairyLight_p, 
      BlueWhite_p, 
      RedWhite_p, 
      Holly_p, 
      RedGreenWhite_p;



enum PALETTES 
{
  RAINBOW_PAL,
  LAVA_PAL,
  ICE_WATER_PAL,
  RAINBOWSTRIPES_PAL,
  FOREST_PAL,
  OCEAN_PAL,
  HEAT_PAL,
  PARTY_PAL,
  CLOUD_PAL,
  ICE_PAL, 
  RETROC9_PAL, 
  SNOW_PAL, 
  FAIRYLIGHT_PAL, 
  BLUEWHITE_PAL, 
  REDWHITHE_PAL, 
  HOLLY_PAL, 
  REDGREENWHITE_PAL,

  NUM_PALETTES
  
};



#define qsubd(x, b)  ((x>b)?b:0)    // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)  // Analog Unsigned subtraction macro. if result <0, then => 0

class WS2812FX {
//class WS2812FX : public Adafruit_NeoPixel {

  typedef uint16_t (WS2812FX::*mode_ptr)(void);
  
  // segment parameters
  public:
    typedef struct segment {
      uint8_t  mode;
      CRGBPalette16 cPalette;
      uint16_t beat88;
      uint16_t start;
      uint16_t stop;
      uint8_t  deltaHue;
      uint16_t hueTime;
      TBlendType blendType;
      bool     reverse;
    } segment;

  // segment runtime parameters
  typedef struct segment_runtime {
    uint32_t timebase;
    uint32_t counter_mode_step;
    unsigned long next_time;
    uint16_t aux_param;
    uint8_t baseHue;
  } segment_runtime;

  public:

    WS2812FX( const uint16_t num_leds, 
              const uint8_t fps = 60, 
              const uint8_t volt = 5, 
              const uint16_t milliamps = 500, 
              const CRGBPalette16 pal = Rainbow_gp, 
              const String Name = "Custom",
              const LEDColorCorrection colc = TypicalLEDStrip  ) {

      leds = new CRGB[num_leds]; //CRGB[NUM_LEDS];

      FastLED.addLeds<WS2812,LED_PIN, GRB>(leds, num_leds);//NUM_LEDS);
      FastLED.setCorrection(colc); //TypicalLEDStrip);
      _currentPalette = CRGBPalette16(CRGB::Black);
      FastLED.setMaxPowerInVoltsAndMilliamps(volt, milliamps);
      
      FastLED.setMaxRefreshRate(fps);
      _fps = fps;
      
      setTargetPalette(pal, Name);

      FastLED.clear(true);
      FastLED.show();

      _mode[FX_MODE_OFF]                     = &WS2812FX::mode_off;
      _mode[FX_MODE_STATIC]                  = &WS2812FX::mode_static;
      _mode[FX_MODE_TWINKLE_EASE]            = &WS2812FX::mode_twinkle_ease;
      _mode[FX_MODE_EASE]                    = &WS2812FX::mode_ease;
      _mode[FX_MODE_MULTI_DYNAMIC]           = &WS2812FX::mode_multi_dynamic;
      _mode[FX_MODE_RAINBOW]                 = &WS2812FX::mode_rainbow;
      _mode[FX_MODE_RAINBOW_CYCLE]           = &WS2812FX::mode_rainbow_cycle;
      _mode[FX_MODE_PRIDE]                   = &WS2812FX::mode_pride;
      _mode[FX_MODE_PRIDE_GLITTER]           = &WS2812FX::mode_pride_glitter;
      _mode[FX_MODE_SCAN]                    = &WS2812FX::mode_scan;
      _mode[FX_MODE_DUAL_SCAN]               = &WS2812FX::mode_dual_scan;
      _mode[FX_MODE_FADE]                    = &WS2812FX::mode_fade;
      _mode[FX_MODE_THEATER_CHASE]           = &WS2812FX::mode_theater_chase;
      _mode[FX_MODE_THEATER_CHASE_DUAL_P]    = &WS2812FX::mode_theater_chase_dual_pal;
      _mode[FX_MODE_THEATER_CHASE_RAINBOW]   = &WS2812FX::mode_theater_chase_rainbow;
      _mode[FX_MODE_TWINKLE_FADE]            = &WS2812FX::mode_twinkle_fade;
      _mode[FX_MODE_TWINKLE_FOX]             = &WS2812FX::mode_twinkle_fox;
      _mode[FX_MODE_SOFTTWINKLES]            = &WS2812FX::mode_softtwinkles;
      _mode[FX_MODE_LARSON_SCANNER]          = &WS2812FX::mode_larson_scanner;
      _mode[FX_MODE_COMET]                   = &WS2812FX::mode_comet;
      _mode[FX_MODE_FIRE_FLICKER]            = &WS2812FX::mode_fire_flicker;
      _mode[FX_MODE_FIRE_FLICKER_SOFT]       = &WS2812FX::mode_fire_flicker_soft;
      _mode[FX_MODE_FIRE_FLICKER_INTENSE]    = &WS2812FX::mode_fire_flicker_intense;
      _mode[FX_MODE_BREATH]                  = &WS2812FX::mode_breath;
      _mode[FX_MODE_RUNNING_LIGHTS]          = &WS2812FX::mode_running_lights;
      _mode[FX_MODE_TWINKLE_NOISEMOVER]      = &WS2812FX::mode_inoise8_mover_twinkle;
      _mode[FX_MODE_NOISEMOVER]              = &WS2812FX::mode_inoise8_mover;
      _mode[FX_MODE_PLASMA]                  = &WS2812FX::mode_plasma;
      _mode[FX_MODE_JUGGLE_PAL]              = &WS2812FX::mode_juggle_pal;
      _mode[FX_MODE_CONFETTI]                = &WS2812FX::mode_confetti;
      _mode[FX_MODE_FILL_BEAT]               = &WS2812FX::mode_fill_beat;
      _mode[FX_MODE_DOT_BEAT]                = &WS2812FX::mode_dot_beat;
      _mode[FX_MODE_TO_INNER]                = &WS2812FX::mode_to_inner;
      _mode[FX_MODE_FILL_BRIGHT]             = &WS2812FX::mode_fill_bright;
      _mode[FX_MODE_FIREWORK]                = &WS2812FX::mode_firework;
      _mode[FX_MODE_FIRE2012]                = &WS2812FX::mode_fire2012WithPalette;
      _mode[FX_MODE_FILL_WAVE]               = &WS2812FX::mode_fill_wave;
      _mode[FX_MODE_BUBBLE_SORT]             = &WS2812FX::mode_bubble_sort;
      _mode[FX_MODE_CUSTOM]                  = &WS2812FX::mode_custom;
      

      _name[FX_MODE_OFF]                        = F("Off");
      _name[FX_MODE_STATIC]                     = F("Static");
      _name[FX_MODE_EASE]                       = F("Ease");
      _name[FX_MODE_TWINKLE_EASE]               = F("Ease Twinkle");
      _name[FX_MODE_BREATH]                     = F("Breath");
      _name[FX_MODE_NOISEMOVER]                 = F("iNoise8 Mover");
      _name[FX_MODE_TWINKLE_NOISEMOVER]         = F("Twinkle iNoise8 Mover");
      _name[FX_MODE_PLASMA ]                    = F("Plasma Effect");
      _name[FX_MODE_JUGGLE_PAL]                 = F("Juggle Moving Pixels");
      _name[FX_MODE_CONFETTI]                   = F("Random Confetti");
      _name[FX_MODE_FILL_BEAT]                  = F("Color Fill Beat");
      _name[FX_MODE_DOT_BEAT]                   = F("Moving Dots");
      _name[FX_MODE_MULTI_DYNAMIC]              = F("Multi Dynamic");
      _name[FX_MODE_RAINBOW]                    = F("Rainbow");
      _name[FX_MODE_RAINBOW_CYCLE]              = F("Rainbow Cycle");
      _name[FX_MODE_PRIDE]                      = F("Pride");
      _name[FX_MODE_PRIDE_GLITTER]              = F("Pride Glitter");
      _name[FX_MODE_SCAN]                       = F("Scan");
      _name[FX_MODE_DUAL_SCAN]                  = F("Dual Scan");
      _name[FX_MODE_FADE]                       = F("Fade");
      _name[FX_MODE_THEATER_CHASE]              = F("Theater Chase");
      _name[FX_MODE_THEATER_CHASE_DUAL_P]       = F("Theater Chase Dual palette");
      _name[FX_MODE_THEATER_CHASE_RAINBOW]      = F("Theater Chase Rainbow");
      _name[FX_MODE_RUNNING_LIGHTS]             = F("Running Lights");
      _name[FX_MODE_TO_INNER]                   = F("Fast to Center");
      _name[FX_MODE_FILL_BRIGHT]                = F("Fill waving Brightness");
      _name[FX_MODE_TWINKLE_FADE]               = F("Twinkle Fade");
      _name[FX_MODE_TWINKLE_FOX]                = F("Twinkle Fox");
      _name[FX_MODE_SOFTTWINKLES]               = F("Soft Twinkles");
      _name[FX_MODE_FIREWORK]                   = F("The Firework");
      _name[FX_MODE_FIRE2012]                   = F("Fire 2012");
      _name[FX_MODE_FILL_WAVE]                  = F("FILL Wave");
      _name[FX_MODE_LARSON_SCANNER]             = F("Larson Scanner");
      _name[FX_MODE_COMET]                      = F("Comet");
      _name[FX_MODE_FIRE_FLICKER]               = F("Fire Flicker");
      _name[FX_MODE_FIRE_FLICKER_SOFT]          = F("Fire Flicker (soft)");
      _name[FX_MODE_FIRE_FLICKER_INTENSE]       = F("Fire Flicker (intense)");
      _name[FX_MODE_BUBBLE_SORT]                = F("Bubble Sort");
      _name[FX_MODE_CUSTOM]                     = F("Custom");

      _pal_name[RAINBOW_PAL]        = F("Rainbow Colors");
      _pal_name[LAVA_PAL]           = F("Lava Colors");
      _pal_name[ICE_WATER_PAL]      = F("Iced Water Colors");
      _pal_name[RAINBOWSTRIPES_PAL] = F("RainbowStripe Colors");
      _pal_name[FOREST_PAL]         = F("Forest Colors");
      _pal_name[OCEAN_PAL]          = F("Ocean Colors");
      _pal_name[HEAT_PAL]           = F("Heat Colors");
      _pal_name[PARTY_PAL]          = F("Party Colors");
      _pal_name[CLOUD_PAL]          = F("Cloud Colors");
      _pal_name[ICE_PAL]            = F("Ice Colors");
      _pal_name[RETROC9_PAL]        = F("Retro C9 Colors");
      _pal_name[SNOW_PAL]           = F("Snow Colors");
      _pal_name[FAIRYLIGHT_PAL]     = F("Fairy Light Colors");
      _pal_name[BLUEWHITE_PAL]      = F("Blue White Colors");
      _pal_name[REDWHITHE_PAL]      = F("Red White Colors");
      _pal_name[HOLLY_PAL]          = F("Holly Colors");
      _pal_name[REDGREENWHITE_PAL]  = F("Red Green White Colors");

      

      _brightness = DEFAULT_BRIGHTNESS;
      _new_mode = 255;
      _running = false;
      _num_segments = 1;
      _segments[0].mode = DEFAULT_MODE;
      _segments[0].cPalette = RainbowColors_p;//DEFAULT_COLOR;
      //_segments[0].colors[1] = CRGBPalette16(CRGB::Black);
      _segments[0].start = 0;
      _segments[0].stop = num_leds - 1;
      _segments[0].beat88 = DEFAULT_BEAT88;
      _segments[0].deltaHue = DEFAULT_DELTAHUE;
      _segments[0].hueTime = DEFAULT_HUETIME;
      _segments[0].blendType = LINEARBLEND;
      RESET_RUNTIME;

      SEGMENT_RUNTIME.timebase = millis();
      SEGMENT_RUNTIME.baseHue += DEFAULT_DELTAHUE;

    }
  

    ~WS2812FX()
    {
      delete leds;
    }

    CRGB *leds;

    void
      init(void),
      service(void),
      start(void),
      stop(void),
      show(void),
      setCurrentPalette(CRGBPalette16 p, String Name),
      setCurrentPalette(uint8_t n),
      setTargetPalette(CRGBPalette16 p, String Name),
      setTargetPalette(uint8_t n),
      map_pixels_palette(uint8_t *hues, uint8_t bright, TBlendType blend),
      setMode(uint8_t m),
      setCustomMode(uint16_t (*p)()),
      setSpeed(uint16_t s),
      increaseSpeed(uint8_t s),
      decreaseSpeed(uint8_t s),
      setColor(uint8_t r, uint8_t g, uint8_t b),
      setColor(uint32_t c),
      setColor(CRGBPalette16 c),
      setBrightness(uint8_t b),
      increaseBrightness(uint8_t s),
      decreaseBrightness(uint8_t s),
      setLength(uint16_t b),
      increaseLength(uint16_t s),
      decreaseLength(uint16_t s),
      trigger(void),
      setNumSegments(uint8_t n),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,   uint16_t beat88, bool reverse),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, CRGBPalette16 pal, uint16_t beat88, bool reverse),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t beat88, bool reverse),
      setBlendType(TBlendType t),
      toggleBlendType(void),
      resetSegments();

    boolean
      isRunning(void);

    uint8_t
      getMode(void),
      getBrightness(void),
      getModeCount(void),
      getPalCount(void),
      getNumSegments(void);

    uint16_t
      getBeat88(void),
      getLength(void);

    uint32_t
      getColor(uint8_t p_index);

    const __FlashStringHelper*
      getModeName(uint8_t m);

    const __FlashStringHelper*
      getPalName(uint8_t p);

    WS2812FX::segment*
      getSegments(void);

    CRGBPalette16 getCurrentPalette(void) { return _currentPalette; };
    CRGBPalette16 getTargetPalette (void)  { return _targetPalette;  };
    
    String getCurrentPaletteName   (void)    { return _currentPaletteName; };
    String getTargetPaletteName    (void)    { return _targetPaletteName;  };

  private:
    void
      strip_off(void),
      fade_out(uint8_t fadeB),
      drawFractionalBar(int pos16, int width, const CRGBPalette16 &pal, uint8_t cindex, uint8_t max_bright),
      coolLikeIncandescent( CRGB& c, uint8_t phase),
      addSparks(uint8_t probability, bool onBlackOnly, bool white);
    
    uint8_t attackDecayWave8( uint8_t i);

    uint16_t
      mode_off(void),
      mode_ease(void),
      mode_twinkle_ease(void),
      mode_ease_func(bool sparks),
      mode_plasma(void),
      mode_fill_wave(void),
      mode_fill_bright(void),
      mode_to_inner(void),
      mode_dot_beat(void),
      mode_fill_beat(void),
      mode_confetti(void),
      mode_juggle_pal(void),
      mode_inoise8_mover_func(bool sparks),
      mode_inoise8_mover(void),
      mode_inoise8_mover_twinkle(void),
      mode_firework(void),

      mode_bubble_sort(void),

      mode_static(void),
      blink(uint32_t, uint32_t, bool strobe),
      mode_blink(void),
      mode_blink_rainbow(void),
      color_wipe(uint32_t, uint32_t, bool),
      mode_multi_dynamic(void),
      mode_breath(void),
      mode_fade(void),
      mode_scan(void),
      mode_dual_scan(void),
      theater_chase(CRGBPalette16 color1, CRGBPalette16 color2),
      //theater_chase(uint32_t, uint32_t),
      mode_theater_chase(void),
      mode_theater_chase_dual_pal(void),
      mode_theater_chase_rainbow(void),
      mode_rainbow(void),
      mode_rainbow_cycle(void),
      pride(bool glitter),
      mode_pride(void),
      mode_pride_glitter(void),
      mode_running_lights(void),
      //twinkle(uint32_t),
      //twinkle_fade(bool),
      mode_twinkle_fade(void),
      mode_sparkle(void),
      mode_larson_scanner(void),
      mode_comet(void),
      fireworks(uint32_t),
      mode_fire_flicker(void),
      mode_fire_flicker_soft(void),
      mode_fire_flicker_intense(void),
      fire_flicker(int),
      mode_fire2012WithPalette(void),
      mode_twinkle_fox( void),
      mode_softtwinkles(void),
      mode_custom(void);

    CRGB computeOneTwinkle( uint32_t ms, uint8_t salt);

    static inline uint16_t 
      triwave16(uint16_t in),
      quadwave16(uint16_t in),
      cubicwave16(uint16_t in),
      ease16InOutQuad( uint16_t i),
      ease16InOutCubic( uint16_t i);



    
    CRGBPalette16 _currentPalette;
    CRGBPalette16 _targetPalette;
    CRGBPalette16 _transitionPalette;

    

    String _currentPaletteName;
    String _targetPaletteName;
    String _transitionPaletteName;
    
    const TProgmemRGBPalette16* _palettes[NUM_PALETTES] =
    {
      &RainbowColors_p,                                   
      &LavaColors_p,                                      
      &Ice_Colors_p, 
      &RainbowStripeColors_p,
      &ForestColors_p, 
      &OceanColors_p, 
      &HeatColors_p, 
      &PartyColors_p,  
      &CloudColors_p,
      &Ice_p, 
      &RetroC9_p, 
      &Snow_p, 
      &FairyLight_p, 
      &BlueWhite_p, 
      &RedWhite_p, 
      &Holly_p, 
      &RedGreenWhite_p
    };

  

    const __FlashStringHelper* _pal_name[NUM_PALETTES]; 
    
    boolean
      _running,
      _transition,
      _triggered;

    uint8_t
      get_random_wheel_index(uint8_t),
      _new_mode,
      _brightness;

    const __FlashStringHelper*
      _name[MODE_COUNT]; // SRAM footprint: 2 bytes per element

    mode_ptr
      _mode[MODE_COUNT]; // SRAM footprint: 4 bytes per element

    uint8_t _segment_index = 0;
    uint8_t _num_segments = 1;
    uint8_t _fps;
    segment _segments[MAX_NUM_SEGMENTS] = { // SRAM footprint: 20 bytes per element
      // mode, color[], speed, start, stop, reverse
      { FX_MODE_STATIC, {CRGBPalette16(RainbowColors_p)}, DEFAULT_BEAT88, 0, 7, false}
    };
    segment_runtime _segment_runtimes[MAX_NUM_SEGMENTS]; // SRAM footprint: 14 bytes per element
};

#endif
