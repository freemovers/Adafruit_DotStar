/* California STEAM ATtiny84a program
 *  
 * Based on the tutorials by Bill Earl on Adafruit:
 * https://learn.adafruit.com/multi-tasking-the-arduino-part-3/using-neopatterns
 *  
 * Each string of LED's are individual controlled by separate data lines, but share clock #4
 * Data lines are defined as follows:
 * 
 * 
 * 
 *                        *
 *                    
 *                        *
 *
 *  *                     *                     *
 *       *                                 *
 *            *           *           *
 *                  3     |      7 
 *                        2
 *                
 *                    5      6
 *             
 *                *             *
 * 
 *            *                    *
 *                         
 *        *                           * 
 * 
 * 
 * 
 * Push buttons are defined as follows:
 * 
 *                 10            9 
 *                        8
 *                
 *                    0      11
 *             
 * 
 */



#include <Adafruit_DotStar84.h>

#define LEGS        5 // Number of legs on the board
#define CLOCKPIN    4 // Clock pin used by all legs

int brightness = 100;

unsigned int Interval = 20;   // milliseconds between updates
unsigned long lastUpdate; // last update of push buttons

bool autoCycle = true;
bool nextPattern = false;
bool previousPattern = false;

uint8_t myPins[] = {2,7,6,5,3}; // Used on the ATtiny84a (additional pins, in order of the legs);

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE, FULL_RAINBOW_CYCLE };
// pattern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class DotstarPatterns : public Adafruit_DotStar
{
    public:

    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    DotstarPatterns(uint16_t pixels, uint8_t dataPin, uint8_t clockPin, uint8_t type, void (*callback)())
    :Adafruit_DotStar(pixels, dataPin, clockPin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {          
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                case FULL_RAINBOW_CYCLE:
                    FullRainbowCycleUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the completion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the completion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 512;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        // The first leg controls the LED in the center of the board as well
        // Calculate the first leg before moving to the others
        for(int i=0; i< numPixels(); i++)
        {
          setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        updatePins(myPins[0], CLOCKPIN);
        show();
        // Calculate the other legs with an offset if -1
        for(int i=1; i< numPixels(); i++)
        {
          setPixelColor(i-1, getPixelColor(i));
        }
        for (uint8_t thisPin = 1; thisPin < LEGS; thisPin++) {
          updatePins(myPins[thisPin], CLOCKPIN);
          show();
        }
        Increment();
    }

        // Initialize for a RainbowCycle
    void FullRainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FULL_RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 512;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void FullRainbowCycleUpdate()
    {
        // The first leg controls the LED in the center of the board as well
        // Calculate the first leg before moving to the others
        for (uint8_t thisPin = 0; thisPin < LEGS; thisPin++) 
        {
          for(int i=0; i< numPixels(); i++)
          {
            setPixelColor(i, Wheel((((i + thisPin*numPixels()) * 5 + Index) & 255)));
          }
          updatePins(myPins[thisPin], CLOCKPIN);
          show();
        }
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels() * 10;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Wheel(Index & 255));
            }
        }
        updatePins(myPins[0], CLOCKPIN);
        show();
        // Calculate the other legs with an offset if -1
        for(int i=1; i< numPixels(); i++)
        {
          setPixelColor(i-1, getPixelColor(i));
        }
        for (uint8_t thisPin = 1; thisPin < LEGS; thisPin++) {
          updatePins(myPins[thisPin], CLOCKPIN);
          show();
        }
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i+1 == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i+1 == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i+1, DimColor(getPixelColor(i)));
            }
        }
        updatePins(myPins[0], CLOCKPIN);
        show();
        // Calculate the other legs with an offset if -1
        for(int i=1; i< numPixels(); i++)
        {
          setPixelColor(i-1, getPixelColor(i));
        }
        for (uint8_t thisPin = 1; thisPin < LEGS; thisPin++) {
          updatePins(myPins[thisPin], CLOCKPIN);
          show();
        }
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimize order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colors are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};

void Ring1Complete();
void AutoCycke();

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
DotstarPatterns Leg1(10, 2, 4, DOTSTAR_BGR, &Ring1Complete);

// Initialize everything and prepare to start
void setup()
{    
    randomSeed(analogRead(0));
    // Initialize all the pixelStrips
    Leg1.begin();
    Leg1.setBrightness(brightness);
    
    // Kick off a pattern
    Leg1.RainbowCycle(3);

    pinMode(8, INPUT_PULLUP);
    pinMode(9, OUTPUT);
    digitalWrite(9, HIGH);
    pinMode(11, INPUT_PULLUP);
    pinMode(0, INPUT_PULLUP);
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
}

// Main loop
void loop()
{ 
    // Update the rings.
    Leg1.Update();
    if((millis() - lastUpdate) > Interval) // time to update
    { 
      lastUpdate = millis();
      if(!digitalRead(11) && brightness<255) Leg1.setBrightness(brightness++);
      if(!digitalRead(0) && brightness>0) Leg1.setBrightness(brightness--);
      if(!digitalRead(8)) autoCycle = true;
      if(!digitalRead(10)) 
      {
        autoCycle = false;
        previousPattern = true;
        Ring1Complete();
      }
      if(!digitalRead(9)) 
      {
        autoCycle = false;
        nextPattern = true;
        Ring1Complete();
      }
    }
}

//------------------------------------------------------------
// Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

void Ring1Complete()
{  
    switch(Leg1.ActivePattern)
    {
        case RAINBOW_CYCLE:          
            if(autoCycle || nextPattern) Leg1.TheaterChase(Leg1.Color(255,255,0), Leg1.Color(0,0,50), 100);
            if(previousPattern) Leg1.Scanner(Leg1.Wheel(random(255)), 100);
            while((!digitalRead(9)) || !digitalRead(10))
            {
              // wait till the push button is released);
              lastUpdate = millis();
            }
            nextPattern = false;
            previousPattern = false;
            break;
        case THEATER_CHASE:
            Leg1.Color1 = Leg1.Wheel(random(255));
            //Leg1.Reverse();
            if(autoCycle || nextPattern) Leg1.FullRainbowCycle(3);
            if(previousPattern) Leg1.RainbowCycle(3);
            while((!digitalRead(9)) || !digitalRead(10))
            {
              // wait till the push button is released);
              lastUpdate = millis();
            }
            nextPattern = false;
            previousPattern = false;
            break;
        case COLOR_WIPE:
            break;
        case SCANNER:
            Leg1.Color1 = Leg1.Wheel(random(255));
            if(autoCycle || nextPattern) Leg1.RainbowCycle(3);
            if(previousPattern) Leg1.TheaterChase(Leg1.Color(255,255,0), Leg1.Color(0,0,50), 100);
            while((!digitalRead(9)) || !digitalRead(10))
            {
              // wait till the push button is released);
              lastUpdate = millis();
            }
            nextPattern = false;
            previousPattern = false;
            break;
        case FADE:
            break;
        case FULL_RAINBOW_CYCLE:          
            if(autoCycle || nextPattern) Leg1.Scanner(Leg1.Wheel(random(255)), 100);
            if(previousPattern) Leg1.TheaterChase(Leg1.Color(255,255,0), Leg1.Color(0,0,50), 100);
            while((!digitalRead(9)) || !digitalRead(10))
            {
              // wait till the push button is released);
              lastUpdate = millis();
            }
            nextPattern = false;
            previousPattern = false;
            break;
        default:
            break;
    }
}
