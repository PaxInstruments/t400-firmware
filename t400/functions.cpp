#define __STDC_LIMIT_MACROS

#include <stdint.h>
#include <avr/io.h>

#include <Arduino.h>
#include "PaxInstruments-U8glib.h" // LCD
#include "typek_constant.h"
#include "t400.h"
#include "functions.h"

namespace Display {

#define TEMP_MAX_VALUE_I    (32760)
#define TEMP_MIN_VALUE_I    (-32760)

#define LINE_COUNT          4

const uint8_t lines[LINE_COUNT][4] = {
    { 0,  7, 132,   7}, // hline between temperatures and status bar
    {31,  0,  31,   7}, // vline between TC1 and TC2
    {65,  0,  65,   7}, // vline between TC2 and TC3
    {99,  0,  99,   7}, // vline between TC3 and TC4
};

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

// Get a reference to the graph point at the specified 
// uint8_t& graphPoint(uint8_t sensor, uint8_t point)
// @param sensor Sensor to use (0-3)
// @param point Index of the graph point to access (0=current point, 1=last point, etc)
// @return reference to the memory address of the specified graph point
#define graphPoint(sensor, point) (graph[sensor][(point + graphCurrentPoint)%MAXIMUM_GRAPH_POINTS])

// const uint8_t yOffset = DISPLAY_HEIGHT - 3;
// Convert a temperature measurement from temperature space to graph space
// uint8_t temperatureToGraphPoint(temperature)
// @param temperature Temperature measurement, in any unit
// @param scale Graph scale, in in degrees per pixel
// @param min Minimum temperature value, in temperature
// @return representation of the temperature in graph space
#define temperatureToGraphPoint(temperature, scale, min) (DISPLAY_HEIGHT - 3 - (temperature-min)/scale*10)
//#define rescaleGraphPoint(point, originalScale, originalMin, newScale, newMin) ((point - ))

// Helper functions
// Prints a fixed point temperture and returns pointer to buffer
#define printtemp(B,T)  (sprintf((B),"%d.%d",((T)/10),((T)%10)),(B))
// Prints an int and returns the pointer to buffer
#define printi(B,I)   (sprintf(buf,"%d",(I)),(B))

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
    // This converts the temperature to a int16_t which is in 1/10ths of
    // a degree
    for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++)
    {
        graph[sensor][(graphCurrentPoint%MAXIMUM_GRAPH_POINTS)] = temperatures[sensor];
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
  if(graphScale<=0) graphScale = 1;

  // graphScale is an int multiplier.  Normally we display 4 temperatures.
  // maxTempInt is the highest temp in the dataset
  // minTempInt is the lowest temp in the dataset

  // Calculate the number of axes digits to display
  axisDigits = 2;
  if((min + (graphScale*4)) > 9999 || min < -999) {
    axisDigits = 4;
  }
  else if((min + (graphScale*4)) > 999 || min < -99) {
    axisDigits = 3;
  }

  return;
}

void setup()
{
  u8g.setContrast(LCD_CONTRAST);    // Set contrast level
  u8g.setRot180();                  // Rotate screen
  u8g.setColorIndex(1);             // Set color mode to binary
  u8g.setFont(u8g_font_5x8r);       // Select font. See https://code.google.com/p/u8glib/wiki/fontsize
  return;
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
  uint8_t lastPoint;
  uint8_t battX = 128;
  uint8_t battY = 9;

  // Update the screen
  u8g.firstPage();
  do {

    // Draw temperature graph
    switch(page){
    case 5:
        u8g.drawLine( 0, 16, 132,  16);    // hline between status bar and graph
        // no break
    default:
      
      // Draw the separator line between axes labels and legend
      u8g.drawLine(CHARACTER_SPACING*axisDigits + 2, DISPLAY_HEIGHT,
                   CHARACTER_SPACING*axisDigits + 2, 18);
    
      // Draw axis labels and marks
      for(uint8_t interval = 0; interval < GRAPH_INTERVALS; interval++)
      {
        u8g.drawPixel(CHARACTER_SPACING*axisDigits + 1, 61 - interval*10);
        sprintf(buf, "%d", (minTempInt/10) + graphScale*interval);
        u8g.drawStr(0, DISPLAY_HEIGHT - interval*10,  buf);
      }
      
       // Calculate how many graph points to display.
      lastPoint = graphPoints;
      
       // If the axis indicies are >2 character length, scale back the graph.
      x = MAXIMUM_GRAPH_POINTS - (axisDigits - 2)*5;
      if(lastPoint > x) lastPoint = x;

      // Draw the temperature graph for each sensor
      for(uint8_t sensor = 0; sensor < 4; sensor++)
      {
        int8_t p;
        float p_float;
        int16_t* starting_point;
        int16_t* wrap_point;

        // if the sensor is out of range, don't show it
        if(temperatures[sensor] == OUT_OF_RANGE_INT || (sensor != graphChannel && graphChannel < 4) )
          continue;

/******************* Float Math Start ********************/
        // Get the latest point from the array
        p_float = ((float)(graphPoint(sensor, 0)))/10.0;
        p = temperatureToGraphPoint(p_float,graphScale, (((float)(minTempInt))/10.0) );
/******************* Float Math End ********************/

        // Draw a string the the latest point
        u8g.drawStr(113+5*sensor, 3 + p, printi(buf,sensor+1));

        // Now, draw all the points
        starting_point = &graph[sensor][graphCurrentPoint];  // Starting address of the graph data
        wrap_point = &graph[sensor][MAXIMUM_GRAPH_POINTS];   // If the address pointer reaches this, reset it to graph[sensor][0]
        for(uint8_t point = 0; point < lastPoint; point++)
        {
/******************* Float Math Start ********************/
          p_float = ((float)(*(starting_point++)))/10.0;
          p = temperatureToGraphPoint(p_float,graphScale, (((float)(minTempInt))/10.0) );
/******************* Float Math End ********************/
          u8g.drawPixel(MAXIMUM_GRAPH_POINTS+12-point,p);
          if(starting_point == wrap_point) {
            starting_point = &graph[sensor][0];
          }
        } // end for point

      }// end for sensor


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
      u8g.drawStr(0*34,   6,  printi(buf,loopcount));
      //u8g.drawStr(1*34,   6,  printi(buf,minTempInt));
      //u8g.drawStr(2*34,   6,  printi(buf,graphScale));
      //u8g.drawStr(3*34,   6,  " ----");
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

} // End namespace Display


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
