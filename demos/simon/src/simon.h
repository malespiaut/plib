#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <math.h>
#include <plib/ssg.h>
#include <GL/glut.h>

#define SI_MAX_MODELS 500

void siUpdate () ;
void siRun () ;
int  siLoad ( char *filename ) ;
void siPosition ( int hh, float x, float y,
                          float z = 0.0f,
                          float h = 0.0f,
                          float p = 0.0f,
                          float r = 0.0f ) ;
 
void siSpeedAndDirection ( int hh, float s, float h, float p = 0.0f ) ;
 
void siVelocity ( int hh, float x, float y,
                          float z = 0.0f,
                          float h = 0.0f,
                          float p = 0.0f,
                          float r = 0.0f ) ;

float siGetPositionX ( int h ) ;
float siGetPositionY ( int h ) ;
float siGetPositionZ ( int h ) ;
float siGetPositionH ( int h ) ;
float siGetPositionP ( int h ) ;
float siGetPositionR ( int h ) ;

float siJoystickLR () ;
float siJoystickUD () ;

bool  siJoystickA () ;
bool  siJoystickB () ;
bool  siJoystickC () ;
bool  siJoystickD () ;
bool  siJoystickL () ;
bool  siJoystickR () ;


