
#ifndef _INCLUDED_SSGPARSER_H_
#define _INCLUDED_SSGPARSER_H_


class _ssgParser
{
  char path[ 256 ];
  FILE* ptr;
  int linenum;
  char linebuf[ 256 ];
  char tokbuf[ 256 ];
  char* tokptr;
  
public :

  int level;

  void openFile( cchar* fname );
  void closeFile();

  char* getLine( s32 startLevel=0 );

  char* parseToken( cchar* name );
  f32 parseFloat( cchar* name );
  s32 parseInt( cchar* name );
  char* parseString( cchar* name );
  
  void expect( cchar* name );
  
  void error( cchar *format, ... );
  void message( cchar *format, ... );
};


#endif