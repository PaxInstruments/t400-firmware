#ifndef FUNCTIONS_H
#define FUNCTIONS_H

extern float GetTypKTemp( float microVolts);

extern void writeNumber(Fat16& file, uint32_t n);

extern void createDataArray(byte graph[100][4], int length);

extern void draw(U8GLIB_LM6063& u8g, float temp1, float temp2, float temp3, float temp4, float ambient, char* fileName, byte graph[100][4], int length);

#endif
