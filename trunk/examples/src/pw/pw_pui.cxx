
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <GL/gl.h>
#include <plib/ul.h>
#include <plib/pw.h>    // This needs to be before <plib/pu.h>
#include <plib/pu.h>

void exFunc ()
{
  fprintf ( stderr, "Exiting.\n" ) ;
  pwCleanup () ;
  exit ( 0 ) ;
}


void rsFunc ( int w, int h )
{
  fprintf ( stderr, "Resized to %d x %d\n", w, h ) ;
}


void mpFunc ( int x, int y )
{
  puMouse ( x, y ) ;
}


void msFunc ( int button, int updn, int x, int y )
{
  puMouse ( button, updn, x, y ) ;
}


void kbFunc ( int key, int updn, int x, int y )
{
  puKeyboard ( key, updn, x, y ) ;
}


void button_cb ( puObject * )
{
  fprintf ( stderr, "Hello World.\n" ) ;
}


int main ( int, char ** )
{
  pwInit ( 100, 100, 640, 480, false, "PLIB Window Demo", true, 0 ) ;
  puInit () ;

  puOneShot *b = new puOneShot ( 50, 50, 200, 80 ) ;

  b -> setLegend   ( "Say Hello" ) ;
  b -> setCallback ( button_cb ) ;

  pwSetCallbacks ( kbFunc, msFunc, mpFunc, rsFunc, exFunc ) ;
  while ( 1 )
  {
    glClearColor ( 0.5, 0.1, 0.1, 1.0 ) ;
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
    puDisplay () ;
    pwSwapBuffers () ;
  }

  return 0;
}


