
#include "ul.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined( WIN32 ) && !defined( __CYGWIN32__ )
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

int *dacioGetBuffer () ;
void dacioInit( int speed, int stereo );
void dacioFlush(void);
void dacioIncomingBufLen(int len);
void dacioGlobalVol(int v);
void dacioOut(void);
void dacioEmpty () ;
void dacioSubtract ( int n ) ;
int  dacioGetLen () ;
unsigned char *dacioGetOutBuffer () ;

#define MAX_OUTRATE	65536
#define MIN_TEMPO	32
#define DEF_OUTRATE	44100
#define DEF_TEMPO	125
#define DEF_SPEED	6


struct InstHirevInfo
{
  unsigned int w;	/* omega: 65536 = 2*pi */
  unsigned int wAcc;	/* w accumlator */
  unsigned char *ptr;
  unsigned char *end;
  unsigned char *loopBeg;
  int x_or;
  int volL;
  int volR;
  int fadeout;
  int lastL;
  int lastR;
} ;

void _MOD_instHirevInit(void);
void _MOD_instHirevSetFrameLen(int l);
void _MOD_instHirevEraseBuf(void);
void _MOD_instHirevFlushBuf(void);
void _MOD_instHirevLoop(InstHirevInfo *ihip);

/* sample info */

struct SampleInfo
{
  unsigned char *beg;
  unsigned char *end;
  int x_or;	/* 0x80 (or 0x8000?) */
  unsigned char *loopBeg;
  int c4spd;
  int vol;
  int mag;	/* magnification */
} ;

void _MOD_instOutRate(unsigned int _or);
void _MOD_instSelectCh(int ch);
void _MOD_instClearPFW(void);
void _MOD_instDoPerFrameWorks(int frame);
void _MOD_instLoop(void);
void _MOD_instInit(void);
void _MOD_instNote(int n, int delay);
void _MOD_instMono(int n);
void _MOD_instVol(int v, int delay);
void _MOD_instTuning(int c4spd);
void _MOD_instSample(SampleInfo *sip, int delay);
void _MOD_instVolSlide(void);
void _MOD_instSetVolSlideParams(int d, int mul, int div, int nthFrame, int fine);
void _MOD_instSetVolSlideFast(int onOff);
void _MOD_instPeriodSlideUp(void);
void _MOD_instPeriodSlideDown(void);
void _MOD_instSetPeriodSlideParams(int speed, int fine);
void _MOD_instSetPeriodAmigaLimit(int onOff);
void _MOD_instPortamento(void);
void _MOD_instSetPortamentoTo(int to);
void _MOD_instSetPortamentoSpeed(int speed);
void _MOD_instSetPortamentoDefaultVol(void);
void _MOD_instSetPortamentoGlissando(int onOff);
void _MOD_instArpeggio(void);
void _MOD_instSetArpeggioParams(int plus1, int plus2);
void _MOD_instRetrig(void);
void _MOD_instSetRetrigParam(int nthFrame);
void _MOD_instSampleOffset(int offset);
void _MOD_instVibrato(void);
void _MOD_instSetVibratoParams(int d, int depth);
void _MOD_instSetVibratoWave(int type, int noRetrig);
void _MOD_instTremolo(void);
void _MOD_instSetTremoloParams(int d, int depth);
void _MOD_instSetTremoloWave(int type, int noRetrig);
void _MOD_instNoteCut(int frame);
void _MOD_instTremor(void);
void _MOD_instSetTremorParams(int onTime, int offTime);
void _MOD_instNoteOff(int delay);
int  _MOD_instIsNoteOff(void);
void _MOD_instPanPosition(int pos);
void _MOD_instEmptyCmd(void);


struct Note
{
  unsigned char note ;
  unsigned char ins  ;
  unsigned char vol  ;
  unsigned char cmd  ;
  unsigned char info ;
} ;

void _MOD_playNoteSetMono(int m);
void _MOD_playNoteSetMasterVol(int mv);
void _MOD_playNoteSetGlobalVol(int gv);
void _MOD_playNoteSetSample(SampleInfo *sip);
void _MOD_playNoteInit(void);
void _MOD_playNoteSetOutRate(int _or);
void _MOD_playNoteSetTempo(int n);
void _MOD_playNoteSetSpeed(int n);
void _MOD_playNoteSetNote(int ch, Note *np);
void _MOD_playNoteSetPatRepeat(int n);
void _MOD_playNote(void);

