/*
 By:Farkad Adnan
 E-mail: farkad.hpfa95@gmail.com
 inst : farkadadnan
 #farkadadnan , #farkad_adnan , فرقد عدنان#
 FaceBook: كتاب عالم الاردوينو
 inst : arduinobook
 #كتاب_عالم_الاردوينو  #كتاب_عالم_الآردوينو 
 */
#include <FastLED.h>
#include <DS3231.h> 

#define PHOTO_RESISTOR_PIN A3
#define LED_PIN     4
#define NUM_LEDS    60
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define FRAMES_PER_SECOND 50
#define MAX_TINT 80
#define MAX_HUE_OFFSET 25
#define MIN_BRIGHTNESS 7
#define MAX_BRIGHTNESS 255
#define MIN_LIGHT 5
#define MAX_LIGHT 300
#define LIGHT_SENSITIVITY 10  //minimum light for light sensor to work as a button
#define NUM_MODES  6
#define MODE_SIMPLE  0
#define MODE_MINIMALISTIC 1
#define MODE_PROGRESS  2
#define MODE_SWEEP  3
#define MODE_SPECTRUM 4
#define MODE_HYSTERICAL 5

CRGB leds[NUM_LEDS];
DS3231 rtc(SDA, SCL);

void setup() {
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps( 5, 1500);
  rtc.begin();
}


void loop()
{
  static int mode = MODE_SIMPLE;
  static unsigned long old_s=0;
  static int old_light = 0;
  static unsigned long last_dimmed = 0;
  static int tint_a = 0;
  static int tint_b = 0;
  static int tintSteps=3;
  static int hueOffset = 0;
  static int hueSteps=1;

  Time t = rtc.getTime();
  unsigned long h=t.hour;
  unsigned long m=t.min;
  unsigned long s=t.sec;
  if(s!=old_s)
  {
    old_s=s;
    int light=analogRead(PHOTO_RESISTOR_PIN);
    if(light < 3 && old_light > LIGHT_SENSITIVITY)
      last_dimmed = millis();
    else if(light > LIGHT_SENSITIVITY && millis() - last_dimmed < 3000)
    {
       mode=(mode+1)%NUM_MODES;
       last_dimmed=0;
    }
    old_light=light;
    light = constrain(light,MIN_LIGHT,MAX_LIGHT);
    int b1=map(light,MIN_LIGHT,MAX_LIGHT,MIN_BRIGHTNESS,MAX_BRIGHTNESS);
    int b2=FastLED.getBrightness();
    if(b1 != b2 && (b1==MIN_BRIGHTNESS || b1==MAX_BRIGHTNESS || abs(b1-b2)*100/b2 > 10))
      FastLED.setBrightness(b1);

    int h_led=h%12*5+m/12;
    int m_led=m;
    int s_led=s;

    hueOffset+=hueSteps;
    if(abs(hueOffset) == MAX_HUE_OFFSET)
      hueSteps*=-1;

    if(tint_a==0)
    {
      tint_b+=tintSteps;
      tint_b=constrain(tint_b,0,MAX_TINT);
      if(tint_b==0 || tint_b==MAX_TINT)
        tintSteps*=-1;
      if(tint_b==0)
        tint_a=1;
    }
    else
    {        
      tint_a+=tintSteps;
      tint_a=constrain(tint_a,0,MAX_TINT);
      if(tint_a==0 || tint_a==MAX_TINT)
        tintSteps*=-1;
    }

    if (mode == MODE_SIMPLE)
      modeSimple(h_led,m_led,s_led,tint_a,tint_b);
    else if (mode == MODE_MINIMALISTIC)
      modeMinimalistic(h_led,m_led,tint_a,tint_b);
    else if (mode == MODE_PROGRESS)
      modeProgress(h_led,m_led,s_led,tint_a,tint_b);
    else if (mode == MODE_SWEEP)
      modeSweep(h_led,m_led,s_led,hueOffset);
    else if (mode == MODE_SPECTRUM)
      modeSpectrum(h_led,m_led,s_led,tint_a,tint_b);
    else if (mode == MODE_HYSTERICAL)
      modeHysterical(h_led,m_led,s_led,hueOffset);
    }
    FastLED.delay(1000/FRAMES_PER_SECOND); 
}


void modeSimple(int h, int m, int s, int tint_a, int tint_b)
{
  FastLED.clear();
  leds[h]+=CRGB(255,tint_a,tint_b);
  leds[m]+=CRGB(tint_b,255,tint_a);
  leds[s]+=CRGB(tint_a,tint_b,255);
  FastLED.show();
}


void modeMinimalistic(int h, int m, int tint_a, int tint_b)
{
  FastLED.clear();
  leds[h]+=CRGB(255,tint_a,tint_b);
  leds[m]+=CRGB(tint_b,255,tint_a);
  FastLED.show();
}


void modeProgress(int h, int m, int s, int tint_a, int tint_b)
{
  FastLED.clear();
  for(int i=h;i>=0 && (i==h || (i!=m && i!=s));i--)
    leds[i]=CRGB(255,tint_a,tint_b);
  for(int i=m;i>=0 && (i==m || (i!=h && i!=s));i--)
    leds[i]=CRGB(tint_b,255,tint_a);
  for(int i=s;i>=0 && (i==s || (i!=h && i!=m));i--)
    leds[i]=CRGB(tint_a,tint_b,255);
  FastLED.show();
}


void modeSweep(int h, int m, int s, int hueOffset)
{
  FastLED.clear();
  for(int i=1;i<15 &&(h-i+60)%60!=m&&(h-i+60)%60!=s;i++)
    leds[(h-i+60)%60]=CHSV((HUE_RED+hueOffset)%255,255,255-15*i);

  for(int i=1;i<15&&(m-i+60)%60!=h&&(m-i+60)%60!=s;i++)
    leds[(m-i+60)%60]=CHSV(HUE_GREEN+hueOffset,255,255-15*i);

  for(int i=1;i<15 &&(s-i+60)%60!=h&&(s-i+60)%60!=m;i++)
    leds[(s-i+60)%60]=CHSV(HUE_BLUE+hueOffset,255,255-15*i);

  leds[h]=CHSV((HUE_RED+hueOffset)%255,255,255);
  leds[m]=CHSV(HUE_GREEN+hueOffset,255,255);
  leds[s]=CHSV(HUE_BLUE+hueOffset,255,255);
  FastLED.show();
}


void modeSpectrum(int h, int m, int s, int tint_a, int tint_b)
{
  FastLED.clear();
  int count=(h-m+60)%60;
  CRGB rgb_h=CRGB(255,tint_a,tint_b);
  CRGB rgb_m=CRGB(tint_b,255,tint_a);
  CRGB rgb_s=CRGB(tint_a,tint_b,255);
  int s_count=(h-s+60)%60;
  if(s_count < count)
  {
    for(int i=0;i<=s_count;i++)
      leds[(h-i+60)%60]= blend(rgb_h,rgb_m, 255*i/count);
    for(int i=s_count;i<=count;i++)
      leds[(h-i+60)%60]= blend(rgb_s,rgb_m, 255*(i-s_count)/(count-s_count));
  }
  else          
  {
    for(int i=0;i<=count;i++)
      leds[(h-i+60)%60]= blend(rgb_h,rgb_m, 255*i/count);
    leds[s]=rgb_s;
  }
  FastLED.show();
}


void modeHysterical(int h, int m, int s, int hueOffset)
{
  FastLED.clear();
  for(int i=4;i<60;i++)
  {
    for(int j=i-4;j<=i;j++)
      leds[(s+j)%60]= CHSV(HUE_BLUE+hueOffset,255,255);
    leds[h]=CHSV((HUE_RED+hueOffset)%255,255,255);
    leds[m]=CHSV(HUE_GREEN+hueOffset,255,255);
    for(int j=0;j<60;j++)
      if(random(10)==1 && j!=h && j!=m )
        leds[j]=leds[j].fadeToBlackBy(100);
    FastLED.show();
    delay(1);        
  }
  for(int i=0;i<30;i++)
  {
    leds[h]=CHSV((HUE_RED+hueOffset)%255,255,255);
    leds[m]=CHSV(HUE_GREEN+hueOffset,255,255);
    leds[s]=CHSV(HUE_BLUE+hueOffset,255,255);
    for(int j=0;j<60;j++)
      if(j!=h && j!=m && j!=s)
        leds[j]=leds[j].fadeToBlackBy(40);
    FastLED.show();
    delay(3);        
  }
  for(int i=0;i<60;i++)
    if(i!=h && i!=m && i!=s)
      leds[i]=0;
  FastLED.show();
}
