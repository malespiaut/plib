
typedef unsigned char ModSample [ 30 ] ;
typedef unsigned char ModNote   [  4 ] ;
struct SampleInfo ;
struct Note       ;

class MODfile
{
  unsigned char *buffer   ;
  unsigned char *p0       ;
  unsigned char *songName ;

  int ordNum ;
  int insNum ;
  int patNum ;
  int  chNum ;
  int rstOrd ;

  unsigned char *ord      ;
  ModSample     *smpInfop ;
  unsigned char *smp0p    ;
  ModNote       *pat      ;
  short         *note     ;
  unsigned char *fileEnd  ;
  unsigned char *repCounter;
  SampleInfo    *sip      ;

  int  firsttime    ;
  int  broken       ;

  int  play_nextOrd ;
  int  play_loopBeg ;
  int  play_loopCnt ;
  int  play_row0    ;
  int  play_row     ;
  int  play_ord0    ;
  int  play_ord     ;

  void makeNoteTable  ( void ) ;
  void tellChSettings ( void ) ;
  int  roundToNote    ( int p ) ;
  void modToS3m       ( ModNote *mp, Note *np ) ;
  void makeSampleInfo ( int smp15 ) ;
  void parseMod       ( unsigned char *pp0, int smp15 ) ;
  void play_one       ( int ppat ) ;
  unsigned char *read_whole_file ( char *fname, int *len ) ;

public:

   MODfile ( char *fname, int speed = 44100, int stereo = 0 ) ;
  ~MODfile () ;

  int update () ;
} ;

