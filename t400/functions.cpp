#define __STDC_LIMIT_MACROS

#include <stdint.h>
#include <avr/io.h>

#include <Arduino.h>
#include "PaxInstruments-U8glib.h" // LCD
#include "typek_constant.h"
#include "t400.h"
#include "functions.h"


#define U8G_PAGE_HEIGHT     8

#define TEMP_MAX_VALUE_I    (32760)
#define TEMP_MIN_VALUE_I    (-32760)

#define LINE_COUNT          4

const uint8_t lines[LINE_COUNT][4] = {
    { 0,  7, 132,   7}, // hline between temperatures and status bar
    {31,  0,  31,   7}, // vline between TC1 and TC2
    {65,  0,  65,   7}, // vline between TC2 and TC3
    {99,  0,  99,   7}, // vline between TC3 and TC4
};

// Function prototypes for t400.ino
int16_t convertTemperatureInt(int16_t celcius);

// Graphical LCD
U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Use HW-SPI
  
// Graph data
int16_t graph[SENSOR_COUNT][MAXIMUM_GRAPH_POINTS]={}; // Array to hold graph data, in temperature values

uint8_t graphCurrentPoint;                           // Index of latest point added to the graph (0,MAXIMUM_GRAPH_POINTS]
uint8_t graphPoints;                                 // Number of valid points to graph

uint32_t graphScale;    // Number of degrees per pixel in the graph[] array.

uint8_t axisDigits;     // Number of digits to display in the axis labels (ex: '80' -> 2, '1000' -> 4, '-999' -> 4)


int16_t minTempInt;
int16_t maxTempInt;

// Helper functions
// Prints an int and returns the pointer to buffer
#define printi(B,I)   (sprintf(buf,"%d",(I)),(B))

char * printtemp(char * buf, int16_t temp)
{
    uint8_t tmp8;
    tmp8 = (uint8_t)abs(temp%10);
    if(temp>9999)
        sprintf(buf,"%d.%d",((temp)/10),tmp8);
    else
        sprintf(buf,"% 4d.%d",((temp)/10),tmp8);
    return buf;
}

void resetGraph()
{
  graphCurrentPoint = 0;
  graphPoints = 0;
  
  graphScale = 1;

  // Blank the array
  for(uint8_t x = 0; x < SENSOR_COUNT; x++)
  {
    for(uint8_t y=0; y < MAXIMUM_GRAPH_POINTS; y++)
    {
        graph[x][y] = OUT_OF_RANGE_INT;
    }
  }

  return;
}

// Update the graph using temperatures[NUM_SENSORS]
void updateGraphData(int16_t* temperatures)
{
    // Increment the current graph point (it wraps around)
    if(graphCurrentPoint == 0)
    {
        graphCurrentPoint = MAXIMUM_GRAPH_POINTS - 1;
    }else{
        graphCurrentPoint -= 1;
    }

    // Increment the number of stored graph points
    if(graphPoints < MAXIMUM_GRAPH_POINTS) {
        graphPoints++;
    }

    // Stick the new temperature in the array
    for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++)
    {
        graph[sensor][graphCurrentPoint] = temperatures[sensor];
    }

  return;
}

void updateGraphScaling()
{
  uint16_t delta;
  int16_t max=TEMP_MIN_VALUE_I;
  int16_t min=TEMP_MAX_VALUE_I;
  int16_t * ptr;
  int16_t p;

  // Itterate over all the data and get the max & min
  for(uint8_t x = 0; x < SENSOR_COUNT; x++)
  {
     ptr = (int16_t*)&graph[x][0];
     for(uint8_t y=0; y < MAXIMUM_GRAPH_POINTS; y++)
     {
       p = *ptr;
       if(p!=OUT_OF_RANGE_INT)
       {
           p = convertTemperatureInt(p);
           if(p>max) max = p;
           if(p<min) min = p;
       }
       ptr++;
     }
  }

  if(max==TEMP_MIN_VALUE_I) max=0;
  if(min==TEMP_MAX_VALUE_I) min=0;

  minTempInt = min;
  maxTempInt = max;
  delta = max - min;
  if(delta<4) maxTempInt=minTempInt+4;

  graphScale = (uint32_t)((delta + 39) / 40);  // TODO: better rounding strategy
  if(graphScale==0) graphScale = 1;

  // graphScale is an int multiplier.  Normally we display 5 temperatures.
  // maxTempInt is the highest temp in the dataset
  // minTempInt is the lowest temp in the dataset

  // Calculate the number of axes digits to display
  axisDigits = 2;
  // These are in 1/10th, is min<-99.0 || max>999.9
  if(min<-999 || (max+(graphScale*4)) >9999) axisDigits = 4;
  else if(min<-100 || (max+(graphScale*4))>999) axisDigits = 3;


  return;
}

void setupDisplay()
{
  u8g.setContrast(LCD_CONTRAST);    // Set contrast level
  u8g.setRot180();                  // Rotate screen
  u8g.setColorIndex(1);             // Set color mode to binary
  u8g.setFont(u8g_font_5x8r);       // Select font. See https://code.google.com/p/u8glib/wiki/fontsize
  return;
}

uint8_t numlength(int16_t num)
{
    if(num>999 || num<-99) return 4;
    if(num>99 || num<-9) return 3;
    if(num>9 || num<0) return 2;
    return 1;
}

uint8_t temperature_to_pixel(int16_t temp)
{
    uint16_t p;
    // This gets the delta between our measurement and the min value (which
    // is the bottom of the chart
    // Example: if minTempInt=300, p_int=325. p = 25, it is 2.5deg higher
    p = (temp -  minTempInt);
    // Now we need to keep all points within the drawing window.  Each temperature step
    // takes up 10 pixels of height (But we are already in 1/10th of degrees!). But if
    // we scaled, scale our value down, this means we need to divide the delta by
    // the scale value
    p = p / graphScale;

    // This gets us a scaled pixel offset.  So at scale 1. 25/1 = 25
    // This means we put the pixel 25 pixels above the low. Since the
    // low is always (DISPLAY_HEIGHT-3) pixels from the top, we take this
    // value and remove our pos.
    p = (DISPLAY_HEIGHT - 3)-p;
    return (uint8_t)p;
}

void draw(
  int16_t* temperatures,
  uint8_t graphChannel,
  uint8_t temperatureUnit,
  char* fileName,
  uint8_t logInterval,
  ChargeStatus::State bStatus,
  uint8_t batteryLevel
  ) {

  // Graphic commands to redraw the complete screen should be placed here
  char buf[8];
  uint8_t page = 0;
  uint8_t x;
  //uint8_t debug_point=0;
  //int16_t debug_point2=0;
  uint8_t num_points;
  uint8_t battX = 128;
  uint8_t battY = 9;

  // Update the screen
  u8g.firstPage();
  do {

    // Each 'page' is a band of 10 pixels across the screen
    // Draw temperature graph
    switch(page){
    case 5:
        u8g.drawLine( 0, 16, 132,  16);    // hline between status bar and graph
        // no break
    default:
    {
        // This runs when we are on page 1-5
        uint8_t p;
        uint8_t index;
        uint8_t bot,top;

        // We only write a horizontal row within a range for each page, this is
        // called 6 times and we loop the whole temperature buffer each time!
        // TODO: Make more efficient

        bot = 63-(page*U8G_PAGE_HEIGHT);
        top = bot-(U8G_PAGE_HEIGHT-1);

        // Draw the separator line between axes labels and legend
        u8g.drawLine(CHARACTER_SPACING*axisDigits + 2, bot,
                   CHARACTER_SPACING*axisDigits + 2, top);

        // TODO: This draw function is called for each page.  The switch
        // statement runs this block of code for pages 0-5.  This means all
        // the code below is run 6 times!  This is super inefficient.
        // Should fix but this is how the u8g library works :-/

        // Draw axis labels and marks
        for(uint8_t interval = 0; interval < GRAPH_INTERVALS; interval++)
        {
            uint8_t spaces=0,x;
            int16_t tmp16;
            u8g.drawPixel(CHARACTER_SPACING*axisDigits + 1, 63-(interval*10)-3);
            tmp16 = (minTempInt/10) + (graphScale*interval);
            // TODO: Write a space string, then over write with number, drrr
            // Add spaces for right justified
            spaces = axisDigits-numlength(tmp16);
            for(x=0;x<spaces;x++)
                sprintf(&(buf[x])," ");
            sprintf(&(buf[spaces]), "%d", tmp16);
            u8g.drawStr(0, DISPLAY_HEIGHT - interval*10,  buf);
        }


        // Calculate how many graph points to display.
        // If the number of axis digits is >2, scale back how many
        // graph points to show
        num_points = graphPoints;
        x = MAXIMUM_GRAPH_POINTS - ((axisDigits - 2)*5);
        if(x<num_points) num_points = x;

        // Draw the temperature graph for each sensor
        for(uint8_t sensor = 0; sensor < 4; sensor++)
        {
            int16_t tmp16;

            // if the sensor is out of range, don't show it. If we are showing one
            // channel, ignore the others
            if(temperatures[sensor] == OUT_OF_RANGE_INT || (sensor != graphChannel && graphChannel < 4) )
              continue;

            tmp16 = convertTemperatureInt(graph[sensor][graphCurrentPoint]);

            // Get the position of the latest point
            //p = temperature_to_pixel(graph[sensor][graphCurrentPoint]);
            p = temperature_to_pixel(tmp16);
#if 0
            if(sensor==3)
            {
                debug_point = p;
                debug_point2 = graph[sensor][graphCurrentPoint];
            }
#endif

            // Draw the channel number at the latest point
            u8g.drawStr(113+5*sensor, 3 + p, printi(buf,sensor+1));

            // Now, draw all the points
            index = graphCurrentPoint;
            for(uint8_t point = 0; point < num_points; point++)
            {
                tmp16 = convertTemperatureInt(graph[sensor][index]);
                //p = temperature_to_pixel(graph[sensor][index]);
                p = temperature_to_pixel(tmp16);
                // Draw pixel at X, Y. X is # of pixels from the left
                u8g.drawPixel(MAXIMUM_GRAPH_POINTS+12-point,p);
                // Go to next pixel
                index++;
                // Wrap when we hit the end of the array
                if(index>=MAXIMUM_GRAPH_POINTS) index = 0;

            } // end for point

        }// end for sensor

    }
    break;

    case 6:
      // Draw status bar
      //u8g.drawStr(0,  15, printi(buf,ambient));         // Ambient temperature
      u8g.drawStr(0,  15, "TypK"); 
      u8g.drawStr(25,  13, "o"); 
      
      switch(temperatureUnit){
      case TEMPERATURE_UNITS_C:
        u8g.drawStr(30, 15, "C");
        break;
      case TEMPERATURE_UNITS_F:
        u8g.drawStr(30, 15, "F");
        break;
      default:
        u8g.drawStr(30, 15, "K");
        break;
      }
      
      // Write file name
      if(fileName==NULL)
          u8g.drawStr(40, 15,"Not logging");
      else
          u8g.drawStr(40, 15,fileName);

      // Interval
      u8g.drawStr( 100, 15, printi(buf,logInterval));
      u8g.drawStr(110, 15, "s");

      // Draw battery
      switch(bStatus){
      case ChargeStatus::DISCHARGING:
        u8g.drawLine(battX,   14, battX+3, 14);
        u8g.drawLine(battX,   14, battX,   10);
        u8g.drawLine(battX+3, 14, battX+3, 10);
        u8g.drawLine(battX+1,  9, battX+2,  9);
      
        // TODO: charge level
        for(uint8_t i = 0; i < batteryLevel; i++) {
          u8g.drawLine(battX, 13-i, battX+3, 13-i);
        }
        break;
      case ChargeStatus::NO_BATTERY:
        u8g.drawLine(battX,   battY,   battX,   battY+5);
        u8g.drawLine(battX,   battY,   battX+3, battY);
        u8g.drawLine(battX,   battY+2, battX+2, battY+2);
        u8g.drawLine(battX,   battY+5, battX+3, battY+5);

        break;
      case ChargeStatus::CHARGING:
        u8g.drawLine(battX,   14, battX+3, 14);
        u8g.drawLine(battX,   14, battX,   10);
        u8g.drawLine(battX+3, 14, battX+3, 10);
        u8g.drawLine(battX+1,  9, battX+2,  9);
      
        static uint8_t batteryState = 0;
        batteryState = (batteryState+1)%5;
        for(uint8_t i = 0; i < batteryState; i++) {
          u8g.drawLine(battX, 13-i, battX+3, 13-i);
        }
        break;
      default:
        // CHARGED
        u8g.drawLine(battX,   battY+1, battX,   battY+5);
        u8g.drawLine(battX+1, battY,   battX+1, battY+5);
        u8g.drawLine(battX+2, battY,   battX+2, battY+5);
        u8g.drawLine(battX+3, battY+1, battX+3, battY+5);
        break;
      }

      break;

    case 7:
      // Draw thermocouple readings
      for (uint8_t i = 0; i < LINE_COUNT; i++)
      {
        const uint8_t* pos = lines[i];
        u8g.drawLine(pos[0], pos[1], pos[2], pos[3]);
      }

      // Display temperature readings  
      #if 1
      for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++)
      {
        if(temperatures[sensor] == OUT_OF_RANGE_INT)
        {
          u8g.drawStr(sensor*34,   6,  " ----");
        }else {
          u8g.drawStr(sensor*34, 6, printtemp(buf,temperatures[sensor]));
        }
      }

      #elif 0
      // DEBUG: Write variable values to the spaces rather than the current temp
      u8g.drawStr(0*34,   6,  printtemp(buf,temperatures[0]));
      u8g.drawStr(1*34,   6,  printtemp(buf,temperatures[1]));
      u8g.drawStr(2*34,   6,  printtemp(buf,temperatures[2]));
      u8g.drawStr(3*34,   6,  printtemp(buf,temperatures[3]));
      #else
      // DEBUG: Write variable values to the spaces rather than the current temp
      u8g.drawStr(0*34,   6,  printi(buf,graphScale));
      u8g.drawStr(1*34,   6,  printi(buf,minTempInt));
      u8g.drawStr(2*34,   6,  printi(buf,debug_point));
      u8g.drawStr(3*34,   6,  printi(buf,debug_point2));
      #endif

      break;

    } // End of select(page)
    
    // Go to next page
    page++;

  }while( u8g.nextPage() );
  
  return;
}

void clear() {
  // Clear the screen
  u8g.firstPage();  
  while( u8g.nextPage() );

  return;
}


// TODO: What namespace is this then?

/******************* Float Math Start ********************/
// This is a lookup from temperature to microvolts
int32_t celcius_to_microvolts(float celcius)
{
  int32_t voltage = 0;
  uint8_t i = 0;
  uint16_t tempLow;
  uint16_t tempHigh;

  // We do this because...?
  i = celcius/10 + 27;

  // This gets us a 10C range this value could be in
  tempLow = lookupThermocouleData(i);
  tempHigh = lookupThermocouleData(i + 1);

  // This interpolates the voltage between the 2 points??
  // ??? (low - offset) + ((temp - VAR) * (delta/10))
  voltage = tempLow - TK_OFFSET + (celcius - (i*10-270)) * (tempHigh - tempLow)/10;

  //return i; // Displays '29.0' on the LCD as expected
  //return tempTypK[29]; // Displays '7256.0'on LCD
  return voltage;
}

// This is a lookup for temperature given microvolts
float microvolts_to_celcius(int32_t microVolts)
{
  float LookedupValue;
  uint16_t tempLow;
  uint16_t tempHigh;

  // Input the junction temperature compensated voltage such that the junction
  // temperature is compensated to 0°C

  //Add an offset for the adjusted lookup table.
  microVolts += TK_OFFSET;

  // Check if it's in range
  if(microVolts > TEMP_TYPE_K_MAX_CONVERSION || microVolts < TEMP_TYPE_K_MIN_CONVERSION)
  {
    return OUT_OF_RANGE;
  }
  
  // Now itterate through the temperature lookup table to find
  // a temperature range for our microvolts.

  // TODO: Binary search here to decrease lookup time
  for(uint16_t i = 0; i<TEMP_TYPE_K_LENGTH; i++)
  {
    tempLow = lookupThermocouleData(i);
    tempHigh = lookupThermocouleData(i + 1);
    
    // NOTE: I think there is a bug here, the lookupThermocouleData macro
    // returns a temperature, but we are comparing that to micovolts
    if(microVolts >= tempLow && microVolts <= tempHigh)
    {
      // Why do we do this math?
      LookedupValue = ((float)-270 + (i)*10) + ((10 *(float)(microVolts - tempLow)) / ((float)(tempHigh - tempLow)));
      break;
    }

  }

  return LookedupValue;
}
/******************* Float Math End ********************/

namespace ChargeStatus {
  
void setup() {
  // Set VBAT_EN high to enable VBAT_SENSE readings
  pinMode(VBAT_EN, OUTPUT);
  digitalWrite(VBAT_EN, HIGH);

  // Configure the batt stat pin as an input
//  analogReference(DEFAULT);
  pinMode(BATT_STAT, INPUT);
  pinMode(VBAT_SENSE, INPUT);  

  // enable the VBUS pad
  USBCON |= (1<<OTGPADE);

  return;
}

State get() {
  // We want to output one of these states:
//   DISCHARGING = 0,    // VBUS=0
//   CHARGING = 1,       // VBUS=1, BATT_STAT<.7V
//   CHARGED = 2,        // VBUS=1, .8V<BATT_STAT<1.2V
//   NO_BATTERY = 3,     // VBUS=1, 3V<BATT_STAT

  bool usbConnected = USBSTA & _BV(VBUS);

  uint16_t battStatCounts = analogRead(BATT_STAT);
  
  #define BATT_CHARGING_COUNTS_MAX  217 // 1024/3.3*.7
  #define BATT_DISCONNECTED_COUNTS_MIN 248 // 1024/3.3*.8
  #define BATT_DISCONNECTED_COUNTS_MAX 372 // 1024/3.3*.1.2

  if(!usbConnected) {
    return DISCHARGING;
  }
  else if(battStatCounts < BATT_CHARGING_COUNTS_MAX) {
    return CHARGING;
  }
  else if(BATT_DISCONNECTED_COUNTS_MIN < battStatCounts
        && battStatCounts < BATT_DISCONNECTED_COUNTS_MAX) {
    return NO_BATTERY;
  }

  // default to this
  return CHARGED;
}

uint8_t getBatteryLevel() {

  // VBAT_SENSE_V= 34 × VBAT/(34 + 18.7)
  // VBAT_SENSE_COUNTS = VBAT_SENSE_V / 3.3 * 1024
  
  #define VBAT_SENSE_FULL 820   // 4.1V
  #define VBAT_SENSE_EMPTY 720  // 3.6V
  
  // Note: We'll divide this into 5 sections so that the user gets a full battery for a little bit.
  uint16_t vbatSenseCounts = analogRead(VBAT_SENSE);
  uint8_t batteryLevel = ((vbatSenseCounts - VBAT_SENSE_EMPTY)*5)/(VBAT_SENSE_FULL - VBAT_SENSE_EMPTY);
  
  return batteryLevel<5?batteryLevel:4;

}

}
