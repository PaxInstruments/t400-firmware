#ifndef FUNCTIONS_H
#define FUNCTIONS_H

extern float GetTypKTemp(float microVolts);

extern void createDataArray(byte graph[100][4], int length);

extern void draw(U8GLIB_LM6063& u8g, float* temperatures, float ambient, char* fileName, byte graph[100][4], int length, int interval);

#endif
