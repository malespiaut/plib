/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/

#ifndef _PU_H_
#define _PU_H_ 1

#include "sg.h"
#include "fnt.h"

/*
  Configuration
*/

#ifndef PU_NOT_USING_GLUT
#define _PU_USE_GLUT   1
#define _PU_USE_GLUT_FONTS   1
#endif

/*
  Include GLUT
 */

#ifndef PU_NOT_USING_GLUT
#  ifdef FREEGLUT_IS_PRESENT
#    include <GL/freeglut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#ifdef PU_NOT_USING_GLUT

#define PU_LEFT_BUTTON          0
#define PU_MIDDLE_BUTTON        1
#define PU_RIGHT_BUTTON         2
#define PU_DOWN                 0
#define PU_UP                   1

#else

#define PU_LEFT_BUTTON      GLUT_LEFT_BUTTON
#define PU_MIDDLE_BUTTON    GLUT_MIDDLE_BUTTON
#define PU_RIGHT_BUTTON     GLUT_RIGHT_BUTTON
#define PU_DOWN             GLUT_DOWN
#define PU_UP               GLUT_UP

#endif

#ifdef _PU_USE_GLUT_FONTS
typedef void *GlutFont ;
#endif

class puFont 
{
protected:
#ifdef _PU_USE_GLUT_FONTS
  GlutFont     glut_font_handle ;
#endif
  fntTexFont * fnt_font_handle ; float pointsize ; float slant ;

public:

  puFont ()
  {
#ifdef _PU_USE_GLUT_FONTS
    glut_font_handle = GLUT_BITMAP_8_BY_13 ;
#endif
    fnt_font_handle  = NULL ;
  }

#ifdef _PU_USE_GLUT_FONTS
  puFont ( GlutFont gfh )
  {
    glut_font_handle = gfh  ;
    fnt_font_handle  = NULL ;
  }
#endif

  puFont ( fntTexFont *tfh, float ps, float sl = 0 )
  {
    initialize ( tfh, ps, sl ) ;
  }

  void initialize ( fntTexFont *tfh, float ps, float sl = 0 )
  {
#ifdef _PU_USE_GLUT_FONTS
    glut_font_handle = (GlutFont) 0 ;
#endif
    fnt_font_handle  = tfh  ;
    pointsize = ps ;
    slant = sl ;
  }

  int getStringDescender () ;
  int getStringHeight    () ;
  int getStringHeight( const char *str ) ;
  int getStringWidth ( const char *str ) ;

  void drawString ( const char *str, int x, int y ) ;
} ;


extern puFont PUFONT_8_BY_13        ;
extern puFont PUFONT_9_BY_15        ;
extern puFont PUFONT_TIMES_ROMAN_10 ;
extern puFont PUFONT_TIMES_ROMAN_24 ;
extern puFont PUFONT_HELVETICA_10   ;
extern puFont PUFONT_HELVETICA_12   ;
extern puFont PUFONT_HELVETICA_18   ;

#define PU_UP_AND_DOWN   254
#define PU_DRAG          255
#define PU_CONTINUAL     PU_DRAG

#define PU_KEY_GLUT_SPECIAL_OFFSET  256

#ifdef PU_NOT_USING_GLUT
#define PU_KEY_F1        (1             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F2        (2             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F3        (3             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F4        (4             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F5        (5             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F6        (6             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F7        (7             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F8        (8             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F9        (9             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F10       (10            + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F11       (11            + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F12       (12            + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_LEFT      (100           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_UP        (101           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_RIGHT     (102           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_DOWN      (103           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_PAGE_UP   (104           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_PAGE_DOWN (105           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_HOME      (106           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_END       (107           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_INSERT    (108           + PU_KEY_GLUT_SPECIAL_OFFSET)

#else
#define PU_KEY_F1        (GLUT_KEY_F1        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F2        (GLUT_KEY_F2        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F3        (GLUT_KEY_F3        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F4        (GLUT_KEY_F4        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F5        (GLUT_KEY_F5        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F6        (GLUT_KEY_F6        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F7        (GLUT_KEY_F7        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F8        (GLUT_KEY_F8        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F9        (GLUT_KEY_F9        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F10       (GLUT_KEY_F10       + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F11       (GLUT_KEY_F11       + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F12       (GLUT_KEY_F12       + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_LEFT      (GLUT_KEY_LEFT      + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_UP        (GLUT_KEY_UP        + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_RIGHT     (GLUT_KEY_RIGHT     + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_DOWN      (GLUT_KEY_DOWN      + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_PAGE_UP   (GLUT_KEY_PAGE_UP   + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_PAGE_DOWN (GLUT_KEY_PAGE_DOWN + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_HOME      (GLUT_KEY_HOME      + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_END       (GLUT_KEY_END       + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_INSERT    (GLUT_KEY_INSERT    + PU_KEY_GLUT_SPECIAL_OFFSET)
#endif

#define PUARROW_UP         0
#define PUARROW_DOWN       1
#define PUARROW_FASTUP     2
#define PUARROW_FASTDOWN   3
#define PUARROW_LEFT       4
#define PUARROW_RIGHT      5
#define PUARROW_FASTLEFT   6
#define PUARROW_FASTRIGHT  7

/* Rational Definitions of PUI Legend and Label Places */
#define PUPLACE_LEFT_TOP          0
#define PUPLACE_TOP_LEFT          0
#define PUPLACE_CENTERED_TOP      1
#define PUPLACE_TOP_CENTERED      1
#define PUPLACE_RIGHT_TOP         2
#define PUPLACE_TOP_RIGHT         2

#define PUPLACE_LEFT_CENTERED     3
#define PUPLACE_CENTERED_LEFT     3
#define PUPLACE_CENTERED_CENTERED 4
#define PUPLACE_RIGHT_CENTERED    5
#define PUPLACE_CENTERED_RIGHT    5

#define PUPLACE_LEFT_BOTTOM       6
#define PUPLACE_BOTTOM_LEFT       6
#define PUPLACE_CENTERED_BOTTOM   7
#define PUPLACE_BOTTOM_CENTERED   7
#define PUPLACE_RIGHT_BOTTOM      8
#define PUPLACE_BOTTOM_RIGHT      8

/* Additional definitions for PUI label places */
#define PUPLACE_ABOVE_LEFT        9
#define PUPLACE_LEFT_ABOVE        9
#define PUPLACE_ABOVE_RIGHT      10
#define PUPLACE_RIGHT_ABOVE      10
#define PUPLACE_BELOW_LEFT       11
#define PUPLACE_LEFT_BELOW       11
#define PUPLACE_BELOW_RIGHT      12
#define PUPLACE_RIGHT_BELOW      12

#define PUPLACE_UPPER_LEFT       13
#define PUPLACE_LEFT_UPPER       13
#define PUPLACE_UPPER_RIGHT      14
#define PUPLACE_RIGHT_UPPER      14
#define PUPLACE_LOWER_LEFT       15
#define PUPLACE_LEFT_LOWER       15
#define PUPLACE_LOWER_RIGHT      16
#define PUPLACE_RIGHT_LOWER      16

/* Keep these for backwards compatibility but deprecate them */
#define PUPLACE_ABOVE           PUPLACE_TOP_LEFT
#define PUPLACE_BELOW           PUPLACE_BOTTOM_LEFT
#define PUPLACE_LEFT            PUPLACE_LOWER_LEFT
#define PUPLACE_RIGHT           PUPLACE_LOWER_RIGHT
#define PUPLACE_CENTERED        PUPLACE_CENTERED_CENTERED
#define PUPLACE_TOP_CENTER      PUPLACE_TOP_CENTERED
#define PUPLACE_BOTTOM_CENTER   PUPLACE_BOTTOM_CENTERED
#define PUPLACE_LEFT_CENTER     PUPLACE_LEFT_CENTERED
#define PUPLACE_RIGHT_CENTER    PUPLACE_RIGHT_CENTERED

#define PUCOL_FOREGROUND 0
#define PUCOL_BACKGROUND 1
#define PUCOL_HIGHLIGHT  2
#define PUCOL_LABEL      3
#define PUCOL_LEGEND     4
#define PUCOL_MAX        5

#define PUSLIDER_CLICK   0
#define PUSLIDER_ALWAYS  1
#define PUSLIDER_DELTA   2

/* These styles may be negated to get 'highlighted' graphics */

#define PUSTYLE_DEFAULT    PUSTYLE_SHADED
#define PUSTYLE_NONE       0
#define PUSTYLE_PLAIN      1
#define PUSTYLE_BEVELLED   2
#define PUSTYLE_BOXED      3
#define PUSTYLE_DROPSHADOW 4
#define PUSTYLE_SPECIAL_UNDERLINED 5
#define PUSTYLE_SMALL_BEVELLED     6
#define PUSTYLE_RADIO      7
#define PUSTYLE_SHADED     8
#define PUSTYLE_SMALL_SHADED   9
#define PUSTYLE_MAX        10

/* These are the gaps that we try to leave around text objects */

#define PUSTR_TGAP   5
#define PUSTR_BGAP   5
#define PUSTR_LGAP   5
#define PUSTR_RGAP   5
#define PUSTR_MAX_HEIGHT  ( 25 + PUSTR_TGAP + PUSTR_BGAP )

#define PU_RADIO_BUTTON_SIZE 16

extern int puRefresh ;

#define PUCLASS_VALUE            0x00000001
#define PUCLASS_OBJECT           0x00000002
#define PUCLASS_GROUP            0x00000004
#define PUCLASS_INTERFACE        0x00000008
#define PUCLASS_FRAME            0x00000010
#define PUCLASS_TEXT             0x00000020
#define PUCLASS_BUTTON           0x00000040
#define PUCLASS_ONESHOT          0x00000080
#define PUCLASS_POPUP            0x00000100
#define PUCLASS_POPUPMENU        0x00000200
#define PUCLASS_MENUBAR          0x00000400
#define PUCLASS_INPUT            0x00000800
#define PUCLASS_BUTTONBOX        0x00001000
#define PUCLASS_SLIDER           0x00002000
#define PUCLASS_DIALOGBOX        0x00004000
#define PUCLASS_ARROW            0x00008000
#define PUCLASS_LISTBOX          0x00010000
#define PUCLASS_DIAL             0x00020000
#define PUCLASS_FILEPICKER       0x00040000
#define PUCLASS_FILESELECTOR     0x00040000 /* Because FilePicker is obsolete */
#define PUCLASS_BISLIDER         0x00080000
#define PUCLASS_TRISLIDER        0x00100000
#define PUCLASS_VERTMENU         0x00200000
#define PUCLASS_LARGEINPUT       0x00400000

/* This function is not required for GLUT programs */
void puSetWindowSize ( int width, int height ) ;
void puSetResizeMode ( int mode ) ;

int  puGetWindow       () ;
void puSetWindow       ( int w ) ;
int  puGetWindowHeight () ;
int  puGetWindowWidth  () ;

class puValue            ;
class puObject           ;
class puGroup            ;
class puInterface        ;
class puButtonBox        ;
class puFrame            ;
class puText             ;
class puButton           ;
class puOneShot          ;
class puPopup            ;
class puPopupMenu        ;
class puMenuBar          ;
class puInput            ;
class puSlider           ;
class puListBox          ;
class puArrowButton      ;
class puDial             ;
class puFilePicker       ;
class puFileSelector     ;
class puBiSlider         ;
class puTriSlider        ;
class puVerticalMenu     ;
class puLargeInput       ;

// Global function to move active object to the end of the "dlist"
// so it is displayed in front of everything else

void puMoveToLast ( puObject *ob ) ;

typedef float puColour [ 4 ] ;  /* RGBA */
typedef puColour puColor ;

struct puBox
{
  int min [ 2 ] ;
  int max [ 2 ] ;

  void draw   ( int dx, int dy, int style, puColour colour[], int am_default, int border ) ;
  void extend ( puBox *bx ) ;

  void empty   ( void ) { min[0]=min[1]=1000000 ; max[0]=max[1]=-1000000 ; }
  int  isEmpty ( void ) { return min[0]>max[0] || min[1]>max[1] ; }
} ;

#define PUSTRING_MAX 80

/* If you change - or add to these, be sure to change _puDefaultColourTable */

extern puColour _puDefaultColourTable[] ;


inline void puSetColour ( puColour dst, puColour src )
{
  dst[0] = src[0] ; dst[1] = src[1] ; dst[2] = src[2] ; dst[3] = src[3] ;
}
inline void puSetColor ( puColour dst, puColour src )
{
  dst[0] = src[0] ; dst[1] = src[1] ; dst[2] = src[2] ; dst[3] = src[3] ;
}

inline void puSetColour ( puColour c, float r, float g, float b, float a = 1.0f )
{
  c [ 0 ] = r ; c [ 1 ] = g ; c [ 2 ] = b ; c [ 3 ] = a ;
}
inline void puSetColor ( puColour c, float r, float g, float b, float a = 1.0f )
{
  c [ 0 ] = r ; c [ 1 ] = g ; c [ 2 ] = b ; c [ 3 ] = a ;
}


void  puInit           ( void ) ;
void  puDisplay        ( void ) ;
void  puDisplay        ( int window_number ) ;
int   puMouse          ( int button, int updown, int x, int y ) ;
int   puMouse          ( int x, int y ) ;
int   puKeyboard       ( int key, int updown ) ;
void  puHideCursor     ( void ) ;
void  puShowCursor     ( void ) ;
int   puCursorIsHidden ( void ) ;
void  puDeleteObject   ( puObject *ob ) ;

// Active widget functions

void puDeactivateWidget () ;
void puSetActiveWidget ( puObject *w, int x, int y ) ;
puObject *puActiveWidget () ;


class puValue
{
protected:
  int   type    ;
  int   integer ;
  float floater ;
  char  *string ;

  int   *res_integer ;
  float *res_floater ;
  char  *res_string  ;

  void re_eval    () ;
  void update_res () ;

public:
  puValue ()
  {
    string = new char [ PUSTRING_MAX ] ;
    type = PUCLASS_VALUE ;
    res_integer = NULL ;
    res_floater = NULL ;
    res_string  = NULL ;
    clrValue () ;
  }

  virtual ~puValue () {  delete string ;  }

  int  getType ( void ) { return type ; }
  char *getTypeString ( void ) ;
  void clrValue ( void ) { setValue ( "" ) ; }

  void setValue ( puValue *pv )
  {
    integer = pv -> integer ;
    floater = pv -> floater ;
    strcpy ( string, pv -> string ) ;
    update_res () ;
    puRefresh = TRUE ;
  }

  void setValuator ( int   *i ) { res_integer = i    ; res_floater = NULL ; res_string = NULL ; re_eval () ; }
  void setValuator ( float *f ) { res_integer = NULL ; res_floater = f    ; res_string = NULL ; re_eval () ; }
  void setValuator ( char  *s ) { res_integer = NULL ; res_floater = NULL ; res_string = s    ; re_eval () ; }

  void setValue ( int   i ) { integer = i ; floater = (float) i ; sprintf ( string, "%d", i ) ; update_res() ; puRefresh = TRUE ; }
  void setValue ( float f ) { integer = (int) f ; floater = f ; sprintf ( string, "%g", f ) ; update_res() ; puRefresh = TRUE ; }
  void setValue ( char *s ) { 
                              if ( s == NULL || s[0] == '\0' )
                              {
                                integer = 0 ;
                                floater = 0.0f ;
                                string[0] = '\0';
                              }
                              else
                              {
                                integer = atoi(s) ;
                                floater = (float)atof(s) ;

                                if ( string != s ) strcpy ( string, s ) ;
                              }
                              update_res () ;
                              puRefresh = TRUE ;
                            }

  void getValue ( int   *i ) { re_eval () ; *i = integer ; }
  void getValue ( float *f ) { re_eval () ; *f = floater ; }
  void getValue ( char **s ) { re_eval () ; *s = string  ; }
  void getValue ( char  *s ) { re_eval () ; strcpy ( s, string ) ; }

  int  getValue ( void ) { re_eval () ; return integer ; }

  float getFloatValue () { re_eval () ; return ( floater ) ; }
  char getCharValue () { re_eval () ; return ( string[0] ) ; }
  char *getStringValue () { return res_string ? res_string : string ; }
} ;

typedef void (*puCallback)(class puObject *) ;
typedef void (*puRenderCallback)(class puObject *, int dx, int dy, void *) ;

void puSetDefaultStyle ( int  style ) ;
int  puGetDefaultStyle ( void ) ;
void puSetDefaultFonts ( puFont  legendFont, puFont  labelFont ) ;
void puGetDefaultFonts ( puFont *legendFont, puFont *labelFont ) ;

puFont puGetDefaultLabelFont  () ;
puFont puGetDefaultLegendFont () ;

void puSetDefaultColourScheme ( float r, float g, float b, float a = 1.0f ) ;
inline void puSetDefaultColorScheme ( float r, float g, float b, float a = 1.0f )
{
  puSetDefaultColourScheme ( r, g, b, a ) ;
}

void puGetDefaultColourScheme ( float *r, float *g, float *b, float *a = NULL );
inline void puGetDefaultColorScheme ( float *r, float *g, float *b, float *a = NULL )
{
  puGetDefaultColourScheme ( r, g, b, a ) ;
}

class puObject : public puValue
{
protected:
  puValue default_value ;

  puBox bbox ;   /* Bounding box of entire Object */
  puBox abox ;   /* Active (clickable) area */
  puColour colour [ PUCOL_MAX ] ;
  puGroup *parent ;

  int active_mouse_edge ; /* is it PU_UP or PU_DOWN (or both) that activates this? */
  int style       ;
  int visible     ;
  int active      ;
  int highlighted ;
  int am_default  ;
  int deactivate_when_leaving ;
  int window ;        /* Which window does the object appear in? */

  const char *label  ; puFont  labelFont ; int labelPlace ;
  const char *legend ; puFont legendFont ; int legendPlace ;

  void *user_data ;
  puCallback cb ;
  puCallback active_cb ;
  puCallback down_cb ;
  puRenderCallback r_cb ;
  void *render_data ;
  int border_thickness ;

  virtual void draw_legend ( int dx, int dy ) ;
  virtual void draw_label  ( int dx, int dy ) ;

public:
  virtual int  isHit ( int x, int y ) { return isVisible() && isActive() &&
                                               x >= abox.min[0] &&
                                               x <= abox.max[0] &&
                                               y >= abox.min[1] &&
                                               y <= abox.max[1] &&
                                               window == puGetWindow () ; }

  virtual void doHit ( int button, int updown, int x, int y ) ;

   puObject ( int minx, int miny, int maxx, int maxy ) ;
  ~puObject () ;

  puObject *next ;
  puObject *prev ;
 
  puBox *getBBox ( void ) { return & bbox ; }
  puBox *getABox ( void ) { return & abox ; }

  void setPosition ( int x, int y )
  {
    if ( abox.isEmpty() )
    {
      abox.max[0] = abox.min[0] = x ;
      abox.max[1] = abox.min[1] = y ;
    }
    else
    {
      abox.max[0] += x - abox.min[0] ;
      abox.max[1] += y - abox.min[1] ;
      abox.min[0]  = x ;
      abox.min[1]  = y ;
    }
    recalc_bbox() ; puRefresh = TRUE ;
  }

  virtual void setSize ( int w, int h )
  {
    abox.max[0] = abox.min[0] + w ;
    abox.max[1] = abox.min[1] + h ;
    recalc_bbox() ; puRefresh = TRUE ;
  }

  void getPosition ( int *x, int *y )
  {
    if ( abox.isEmpty () )
    {
      if ( x ) *x = 0 ;
      if ( y ) *y = 0 ;
    }
    else
    {
      if ( x ) *x = abox.min[0] ;
      if ( y ) *y = abox.min[1] ;
    }
  }

  void getSize ( int *w, int *h )
  {
    if ( abox.isEmpty () )
    {
      if ( w ) *w = 0 ;
      if ( h ) *h = 0 ;
    }
    else
    {
      if ( w ) *w = abox.max[0] - abox.min[0] ;
      if ( h ) *h = abox.max[1] - abox.min[1] ;
    }
  }

  virtual void recalc_bbox ( void ) ;
  virtual int  checkHit ( int button, int updown, int x, int y ) ;
  virtual int  checkKey ( int key   , int updown ) ;
  virtual void draw ( int dx, int dy ) = 0 ;

  puGroup     *getParent     ( void ) { return parent ; }
  void        setParent      ( puGroup* p ) { parent = p ; }
  puObject    *getNextObject ( void ) { return next   ; }
  puObject    *getPrevObject ( void ) { return prev   ; }

  void       setCallback ( puCallback c ) { cb = c ;    }
  puCallback getCallback ( void )               { return cb ; }
  void       invokeCallback ( void ) { if ( cb ) (*cb)(this) ; }

  void       setActiveCallback ( puCallback c ) { active_cb = c ;    }
  puCallback getActiveCallback ( void )               { return active_cb ; }
  void       invokeActiveCallback ( void ) { if ( active_cb ) (*active_cb)(this) ; }

  void       setDownCallback ( puCallback c ) { down_cb = c ;    }
  puCallback getDownCallback ( void )               { return down_cb ; }
  virtual void invokeDownCallback ( void ) { if ( down_cb ) (*down_cb)(this) ; }

  void       setRenderCallback ( puRenderCallback c, void *d = NULL ) { r_cb = c ; render_data = d ; }
  puRenderCallback getRenderCallback ( void ) { return r_cb ; }
  void      *getRenderCallbackData ( void ) { return render_data ; }
  void       invokeRenderCallback ( int dx, int dy ) { if ( r_cb ) (*r_cb)(this, dx, dy, render_data) ; }

  virtual void setBorderThickness ( int t )  {  border_thickness = t ;  }
  int getBorderThickness ( void )  {  return border_thickness ;  }

  void  makeReturnDefault ( int def ) { am_default = def ; }
  int   isReturnDefault   ( void )          { return am_default ; }

  void setDeactivateWhenLeaving ( int i )  {  deactivate_when_leaving = i ;  }
  int     deactivateWhenLeaving ( void )  {  return deactivate_when_leaving ;  }

  int   getWindow () { return window ; }
  void  setWindow ( int w ) { window = w ; }

  void  setActiveDirn ( int e ) { active_mouse_edge = e ; }
  int   getActiveDirn ( void ) { return active_mouse_edge ; }

  void  setLegend ( const char *l ) { legend = l ; recalc_bbox() ; puRefresh = TRUE ; }
  const char *getLegend ( void ) const { return legend ; }

  void  setLegendFont ( puFont f ) { legendFont = f ; recalc_bbox() ; puRefresh = TRUE ; }
  puFont getLegendFont ( void ) { return legendFont ; }

  void  setLegendPlace ( int lp ) { legendPlace = lp ; recalc_bbox() ; puRefresh = TRUE ; }
  int   getLegendPlace ( void ) { return legendPlace ; }

  void  setLabel ( const char *l ) { label = l ; recalc_bbox() ; puRefresh = TRUE ; }
  const char *getLabel ( void ) const { return label ; }

  void  setLabelFont ( puFont f ) { labelFont = f ; recalc_bbox() ; puRefresh = TRUE ; }
  puFont getLabelFont ( void ) { return labelFont ; }

  void  setLabelPlace ( int lp ) { labelPlace = lp ; recalc_bbox() ; puRefresh = TRUE ; }
  int   getLabelPlace ( void ) { return labelPlace ; }

  void activate   ( void ) { if ( ! active  ) { active  = TRUE  ; puRefresh = TRUE ; } }
  void greyOut    ( void ) { if (   active  ) { active  = FALSE ; puRefresh = TRUE ; } }
  int  isActive   ( void ) { return active ; }

  void highlight  ( void ) { if ( ! highlighted ) { highlighted = TRUE  ; puRefresh = TRUE ; } }
  void lowlight   ( void ) { if (   highlighted ) { highlighted = FALSE ; puRefresh = TRUE ; } }
  int isHighlighted( void ){ return highlighted ; }

  void reveal     ( void ) { if ( ! visible ) { visible = TRUE  ; puRefresh = TRUE ; } }
  void hide       ( void ) { if (   visible ) { visible = FALSE ; puRefresh = TRUE ; } }
  int  isVisible  ( void ) { return visible ; }

  virtual void setStyle ( int which )
  {
    style = which ;
    recalc_bbox () ;
    puRefresh = TRUE ;
  }

  int  getStyle ( void ) { return style ; }

  void setColourScheme ( float r, float g, float b, float a = 1.0f ) ;
  void setColorScheme ( float r, float g, float b, float a = 1.0f )
  {
    setColourScheme ( r, g, b, a ) ;
  }

  virtual void setColour ( int which, float r, float g, float b, float a = 1.0f )
  {
    puSetColour ( colour [ which ], r, g, b, a ) ;
    puRefresh = TRUE ;
  }
  virtual void setColor ( int which, float r, float g, float b, float a = 1.0f )
  {
    setColour ( which, r, g, b, a ) ;
  }

  void getColour ( int which, float *r, float *g, float *b, float *a = NULL )
  {
    if ( r ) *r = colour[which][0] ;
    if ( g ) *g = colour[which][1] ;
    if ( b ) *b = colour[which][2] ;
    if ( a ) *a = colour[which][3] ;
  }
  void getColor ( int which, float *r, float *g, float *b, float *a = NULL )
  {
    getColour ( which, r, g, b, a );
  }

  void  setUserData ( void *data ) { user_data = data ; }
  void *getUserData ( void )             { return user_data ; }

  void defaultValue ( void ) { setValue ( & default_value ) ; }

  void setDefaultValue ( int    i ) { default_value.setValue ( i ) ; }
  void setDefaultValue ( float  f ) { default_value.setValue ( f ) ; }
  void setDefaultValue ( char  *s ) { default_value.setValue ( s ) ; }

  void getDefaultValue ( int   *i ) { default_value.getValue ( i ) ; }
  void getDefaultValue ( float *f ) { default_value.getValue ( f ) ; }
  void getDefaultValue ( char **s ) { default_value.getValue ( s ) ; }
  int  getDefaultValue ( void )    { return default_value.getValue () ; }
} ;

/*
  The 'live' interface stack is used for clicking and rendering.
*/

void         puPushLiveInterface        ( puInterface *in ) ;
void         puPopLiveInterface         ( puInterface *in = 0 ) ;
int          puNoLiveInterface          ( void ) ;
puInterface *puGetBaseLiveInterface     ( void ) ;
puInterface *puGetUltimateLiveInterface ( void ) ;

/*
  The regular group stack is used for adding widgets
*/

void     puPushGroup    ( puGroup *in ) ;
void     puPopGroup     ( void ) ;
int      puNoGroup      ( void ) ;
puGroup *puGetCurrGroup ( void ) ;

class puGroup : public puObject
{
protected:
  int num_children ;
  puObject *dlist ;

  int mouse_x ;    // Coordinates of mouse when right button pressed for
  int mouse_y ;    // drag and drop

  short mouse_active;  // Flag telling whether interface is presently being dragged

  void doHit       ( int button, int updown, int x, int y ) ;

  short floating;   // Flag telling whether the interface floats in the window or stays put

public:

  puGroup ( int x, int y ) : puObject ( x, y, x, y )
  {
    type |= PUCLASS_GROUP ;
    dlist = NULL ;
    num_children = 0 ;
    mouse_x = 0 ;
    mouse_y = 0 ;
    mouse_active = FALSE ;
    floating = FALSE ;
    puPushGroup ( this ) ;
  }

  ~puGroup () ;

  void recalc_bbox ( void ) ;
  virtual void add    ( puObject *new_object ) ;
  virtual void remove ( puObject *old_object ) ;
  virtual void empty ( void ) ;

  void draw        ( int dx, int dy ) ;
  int  checkHit    ( int button, int updown, int x, int y ) ;
  int  checkKey    ( int key   , int updown ) ;

  puObject *getFirstChild ( void ) { return dlist ; }
  int getNumChildren ( void ) { return num_children ; }

  virtual void close ( void )
  {
    if ( puGetCurrGroup () != this )
      ulSetError ( UL_WARNING, "PUI: puGroup::close() is mismatched!" ) ;
    else
      puPopGroup () ;
  }

  void setFloating (short value) { floating = value; }
  short getFloating () { return floating; }

  void setChildStyle ( int childs, int style, int recursive = FALSE ) ;
  void setChildBorderThickness ( int childs, int t, int recursive = FALSE ) ;

  void setChildColour ( int childs, int which,
                        float r, float g, float b, float a = 1.0f,
                        int recursive = FALSE ) ;
  void setChildColor ( int childs, int which,
                       float r, float g, float b, float a = 1.0f,
                       int recursive = FALSE )
  {
    setChildColour ( childs, which, r, g, b, a, recursive ) ;
  }

  void setChildColourScheme ( int childs,
                              float r, float g, float b, float a = 1.0f,
                              int recursive = FALSE ) ;
  void setChildColorScheme ( int childs,
                             float r, float g, float b, float a = 1.0f,
                             int recursive = FALSE )
  {
    setChildColourScheme ( childs, r, g, b, a, recursive ) ;
  }
} ;


class puInterface : public puGroup
{
public:

  puInterface ( int x, int y ) : puGroup ( x, y )
  {
    type |= PUCLASS_INTERFACE ;
    puPushLiveInterface ( this ) ;
  }

  ~puInterface () ;
} ;


class puFrame : public puObject
{
public:
  void draw ( int dx, int dy ) ;
  puFrame ( int minx, int miny, int maxx, int maxy ) :
             puObject ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_FRAME ;
  }

  void doHit ( int button, int updown, int x, int y )
  {
    if ( puActiveWidget() && ( this != puActiveWidget() ) )
    {
      // Active widget exists and is not this one; call its down callback if it exists

      puActiveWidget() -> invokeDownCallback () ;
      puDeactivateWidget () ;
    }

    if ( isHit ( x, y ) && ( updown != PU_DRAG ) )
      puMoveToLast ( this -> parent );
  }
} ;



class puText : public puObject
{
public:
  virtual int  isHit ( int /* x */, int /* y */ ) { return FALSE ; }
  void draw ( int dx, int dy ) ;
  puText ( int x, int y ) : puObject ( x, y, x, y )
  {
    type |= PUCLASS_TEXT ;
  }
} ;


class puButton : public puObject
{
protected:
public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puButton   ( int minx, int miny, const char *l ) :
                 puObject ( minx, miny,
                            minx + puGetDefaultLegendFont().getStringWidth ( l ) + PUSTR_LGAP + PUSTR_RGAP,
                            miny + puGetDefaultLegendFont().getStringHeight () + puGetDefaultLegendFont().getStringDescender () + PUSTR_TGAP + PUSTR_BGAP )
  {
    type |= PUCLASS_BUTTON ;
    setLegend ( l ) ;
  }

  puButton   ( int minx, int miny, int maxx, int maxy ) :
                 puObject ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_BUTTON ;
  }
} ;


class puOneShot : public puButton
{
protected:
public:
  void doHit ( int button, int updown, int x, int y ) ;

  puOneShot ( int minx, int miny, const char *l ) : puButton   ( minx, miny, l )
  {
    type |= PUCLASS_ONESHOT ;
  }

  puOneShot ( int minx, int miny, int maxx, int maxy ) :
                 puButton ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_ONESHOT ;
  }
} ;



class puArrowButton : public puOneShot
{
protected:

  int arrow_type ;

public:
  void draw  ( int dx, int dy ) ;

  int  getArrowType ( void  ) { return arrow_type ; }
  void setArrowType ( int i ) { arrow_type = i ; }

  puArrowButton ( int minx, int miny, int maxx, int maxy, int ptype ) :
                 puOneShot ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_ARROW ;
    arrow_type = ptype ;
  }
} ;


class puSlider : public puObject
{
protected:
  int vert ;
  float last_cb_value ;
  float cb_delta ;
  int   cb_mode ;
  float slider_fraction ;
  void draw_slider_box ( int dx, int dy, float val, char *box_label = NULL ) ;
public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puSlider ( int minx, int miny, int sz, int vertical = FALSE ) :
     puObject ( minx, miny, vertical ?
	       ( minx + puGetDefaultLegendFont().getStringWidth ( "W" ) +
			PUSTR_LGAP + PUSTR_RGAP ) :
	       ( minx + sz ),
	      vertical ?
	       ( miny + sz ) :
	       ( miny + puGetDefaultLegendFont().getStringHeight () +
			puGetDefaultLegendFont().getStringDescender () +
			PUSTR_TGAP + PUSTR_BGAP )
	     )
  {
    type |= PUCLASS_SLIDER ;
    slider_fraction = 0.1f ;
    getValue ( & last_cb_value ) ;
    vert = vertical ;
    cb_delta = 0.1f ;
    cb_mode = PUSLIDER_ALWAYS ;
  }

  /* Blake Friesen - alternate constructor which lets you explicitly set width */
  puSlider ( int minx, int miny, int sz, int vertical, int width ) :
     puObject ( minx, miny, vertical ?
                             ( minx + width ) :
                             ( minx + sz ),
                            vertical ?
                             ( miny + sz ) :
                             ( miny + width ) 
                           )
  {
    type |= PUCLASS_SLIDER ;
    slider_fraction = 0.1f ;
    getValue ( & last_cb_value ) ;
    vert = vertical ;
    cb_delta = 0.1f ;
    cb_mode = PUSLIDER_ALWAYS ;
  }

  void setCBMode ( int m ) { cb_mode = m ; }
  float getCBMode ( void ) { return (float)cb_mode ; }

  int  isVertical ( void ) { return vert ; }

  void setDelta ( float f ) { cb_delta = (f<=0.0f) ? 0.1f : (f>=1.0f) ? 0.9f : f ; }
  float getDelta ( void ) { return cb_delta ; }

  void setSliderFraction ( float f ) { slider_fraction = (f<=0.0f) ? 0.1f : (f>=1.0f) ? 0.9f : f ; }
  float getSliderFraction ( void ) { return slider_fraction ; }
} ;


class puBiSlider : public puSlider
{
protected:
  int max_value ;
  int min_value ;

  int current_max ;
  int current_min ;

  int active_button ;  // Zero for none, one for min, two for max
public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puBiSlider ( int minx, int miny, int sz, int vertical = FALSE ) :
     puSlider ( minx, miny, sz, vertical )
  {
    type |= PUCLASS_BISLIDER ;
    max_value = 1 ;
    min_value = 0 ;
    current_max = 1 ;
    current_min = 0 ;
    active_button = 0 ;
  }

  void setMaxValue ( int i )
  {
    max_value = i ;
    slider_fraction = 1.0f / (float)( max_value-min_value+1 ) ;
  }

  int getMaxValue ( void ) { return max_value ; }

  void setMinValue ( int i )
  {
    min_value = i ;
    slider_fraction = 1.0f / (float)( max_value-min_value+1 ) ;
  }

  int getMinValue ( void ) { return min_value ; }

  void setCurrentMax ( int i ) { current_max = i ; }
  int getCurrentMax ( void ) { return current_max ; }

  void setCurrentMin ( int i ) { current_min = i ; }
  int getCurrentMin ( void ) { return current_min ; }

  void setActiveButton ( int i ) { active_button = i ; }
  int getActiveButton ( void ) { return active_button ; }
} ;



class puTriSlider : public puBiSlider
{
protected:
  // "active_button" is now zero for none, one for min, two for middle, three for max
  int freeze_ends ;  // true to make end sliders unmovable
public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puTriSlider ( int minx, int miny, int sz, int vertical = FALSE ) :
     puBiSlider ( minx, miny, sz, vertical )
  {
    type |= PUCLASS_TRISLIDER ;
    freeze_ends = TRUE ;
  }

   int getFreezeEnds () {  return freeze_ends ;  }
   void setFreezeEnds ( int val ) {  freeze_ends = val ;  }
} ;



class puListBox : public puButton
{
protected:
  char ** list ;
  int num ;
  int top ;

public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puListBox  ( int minx, int miny, int maxx, int maxy, char** list = NULL ) ;

  void newList     ( char ** _list ) ;
  int  getNumItems () const { return num ; }
  int  getTopItem  () const { return top ; }
  void setTopItem  ( int item_index ) ;
} ;


class puDial : public puSlider
{
protected:
  int wrap ;  // Flag telling whether you can wrap around the bottom of the dial
public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puDial ( int minx, int miny, int sz ) :
     puSlider ( minx, miny, sz )
  {
    type |= PUCLASS_DIAL ;
    setValue ( 0.0f ) ;
    wrap = TRUE ;
    setSize ( sz, sz ) ; /* Override the funky math that the base slider did! */
  }

  void setWrap ( int in )  {  wrap = in ;  }
  int getWrap ( void )  {  return wrap ;  }
} ;



class puPopup : public puInterface
{
protected:
public:
  puPopup ( int x, int y ) : puInterface ( x, y )
  {
    type |= PUCLASS_POPUP ;
    hide () ;
  }
} ;


class puPopupMenu : public puPopup
{
protected:
public:
  puPopupMenu ( int x, int y ) : puPopup ( x, y )
  {
    type |= PUCLASS_POPUPMENU ;
  }

  puObject *add_item ( const char *str, puCallback cb ) ;
  int  checkHit ( int button, int updown, int x, int y ) ;
  int  checkKey ( int key   , int updown ) ;
  void close ( void ) ;
} ;


class puMenuBar : public puInterface
{
protected:
public:
  puMenuBar ( int h = -1 ) :
         puInterface ( 0, h < 0 ? puGetWindowHeight() -
                      ( puGetDefaultLegendFont().getStringHeight () + PUSTR_TGAP + PUSTR_BGAP ) : h )
  {
    type |= PUCLASS_MENUBAR ;
  }

  void add_submenu ( const char *str, char *items[], puCallback cb[] ) ;
  void close ( void ) ;
} ;


class puVerticalMenu : public puGroup
{
protected:
public:
  puVerticalMenu ( int x = -1, int y = -1 ) :

  puGroup ( x < 0 ? puGetWindowWidth() -
                     ( puGetDefaultLegendFont().getStringWidth ( " " )
                       + PUSTR_TGAP + PUSTR_BGAP ) : x,
            y < 0 ? puGetWindowHeight() -
                     ( puGetDefaultLegendFont().getStringHeight ()
                       + PUSTR_TGAP + PUSTR_BGAP ) : y)
  {
    type |= PUCLASS_VERTMENU ;
    floating = TRUE ;
  }

  void add_submenu ( char *str, char *items[], puCallback cb[] ) ;
  void close ( void ) ;
} ;


class puInput : public puObject
{
protected:
  int accepting ;
  int cursor_position ;
  int select_start_position ;
  int select_end_position ;
  char *valid_data ;

  void normalize_cursors ( void ) ;
  void removeSelectRegion ( void ) ;

  int input_disabled ;

public:
  void draw     ( int dx, int dy ) ;
  int checkHit ( int button, int updown, int x, int y )
  {
    if ( input_disabled ) return FALSE ;
    return puObject::checkHit ( button, updown, x, y ) ;
  }

  void doHit    ( int button, int updown, int x, int y ) ;
  int  checkKey ( int key, int updown ) ;

  int  isAcceptingInput ( void ) { return accepting ; }
  void rejectInput      ( void ) { accepting = FALSE ; }

  void acceptInput ( void )
  {
    accepting = TRUE ;
    cursor_position = strlen ( getStringValue() ) ;
    select_start_position = select_end_position = -1 ;
  }

  int  getCursor ( void )  { return cursor_position ; }
  void setCursor ( int c ) { cursor_position = c ; }

  void setSelectRegion ( int s, int e )
  {
    select_start_position = s ;
    select_end_position   = e ;
  }

  void getSelectRegion ( int *s, int *e )
  {
    if ( s ) *s = select_start_position ;
    if ( e ) *e = select_end_position   ;
  }

  char *getValidData () { return valid_data ; }
  void setValidData ( char *data )
  {
    if ( valid_data )
    {
      delete valid_data ;
      valid_data = NULL ;
    }

    if ( data )
    {
      valid_data = new char [ strlen ( data ) + 1 ] ;
      strcpy ( valid_data, data ) ;
    }
  }

  void addValidData ( char *data )
  {
    int new_data_len = 1 ;
    if ( valid_data ) new_data_len += strlen ( valid_data ) ;
    if ( data )       new_data_len += strlen ( data ) ;
    char *new_data = new char [ new_data_len ] ;
    strcpy ( new_data, "\0" ) ;
    if ( valid_data ) strcat ( new_data, valid_data ) ;
    if ( data )       strcat ( new_data, data ) ;
    delete valid_data ;
    valid_data = new_data ;
  }

  int isValidCharacter ( char c )
  {
    return ( ( strchr ( valid_data, c ) != NULL ) ? 1 : 0 ) ;
  }

  puInput ( int minx, int miny, int maxx, int maxy ) :
             puObject ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_INPUT ;

    accepting = FALSE ;

    cursor_position       =  0 ;
    select_start_position = -1 ;
    select_end_position   = -1 ;

    valid_data = NULL ;
    input_disabled = FALSE ;

    setColourScheme ( 0.8f, 0.7f, 0.7f ) ; /* Yeukky Pink */
  }

  ~puInput ()
  {
    if ( valid_data ) free ( valid_data ) ;
  }

  virtual void invokeDownCallback ( void )
  {
    rejectInput () ;
    normalize_cursors () ;
    if ( down_cb ) (*down_cb)(this) ;
  }

  void enableInput ()  {  input_disabled = FALSE ;  }
  void disableInput () {  input_disabled = TRUE ;  }
  int  inputDisabled ()  {  return input_disabled ;  }
} ;


class puButtonBox : public puObject
{
protected:
  int one_only ;
  int num_kids ;
  char **button_labels ;

public:

  puButtonBox ( int minx, int miny, int maxx, int maxy, 
                char **labels, int one_button ) ;

  int isOneButton ( void ) { return one_only ; }

  int checkKey ( int key   , int updown ) ;
  int checkHit ( int button, int updown, int x, int y ) ;
  void draw    ( int dx, int dy ) ;
} ;


class puDialogBox : public puPopup
{
protected:
public:

  puDialogBox ( int x, int y ) : puPopup ( x, y )
  {
    type |= PUCLASS_DIALOGBOX ;
  }
} ;



class puFilePicker : public puDialogBox
{
protected:
  char** files ;
  char*  dflag ;
  int num_files   ;
  int arrow_count ;

  char startDir [ PUSTRING_MAX ] ;

  void find_files () ;
  static void handle_select ( puObject* ) ;
  static void input_entered ( puObject* ) ;

  puFrame   *frame         ;
  puListBox *list_box      ;
  puSlider  *slider        ;
  puOneShot *cancel_button ;
  puOneShot *ok_button     ;
  puArrowButton *up_arrow       ;
  puArrowButton *down_arrow     ;
  puArrowButton *fastup_arrow   ;
  puArrowButton *fastdown_arrow ;

  void puFilePickerInit ( int x, int y, int w, int h,
                          int arrows, const char *dir, const char *title ) ;

public:

/*******THIS CLASS IS OBSOLETE ********
  Please use puFileSelector - puFilePicker
  is obsolete and will be removed in a 
  future release.
 *************************************/

  puFilePicker ( int x, int y, int w, int h, int arrows,
                 const char *dir, const char *title = "Pick a file" ) ;
  puFilePicker ( int x, int y, int w, int h,
                 const char *dir, const char *title = "Pick a file" ) ;
  puFilePicker ( int x, int y, int arrows,
                 const char* dir, const char *title = "Pick a file" ) ;
  puFilePicker ( int x, int y,
                 const char* dir, const char *title = "Pick a file" ) ;

  ~puFilePicker () ;

  void setSize ( int w, int h ) ;
} ;


class puFileSelector : public puDialogBox
{
protected:
  char** files ;
  char*  dflag ;
  int num_files   ;
  int arrow_count ;

  char startDir [ PUSTRING_MAX ] ;

  void find_files () ;
  static void handle_select ( puObject* ) ;
  static void input_entered ( puObject* ) ;

  puFrame   *frame         ;
  puListBox *list_box      ;
  puSlider  *slider        ;
  puOneShot *cancel_button ;
  puOneShot *ok_button     ;
  puInput   *input         ;
  puArrowButton *up_arrow       ;
  puArrowButton *down_arrow     ;
  puArrowButton *fastup_arrow   ;
  puArrowButton *fastdown_arrow ;

  void puFileSelectorInit ( int x, int y, int w, int h,
                          int arrows, const char *dir, const char *title ) ;

public:

  puFileSelector ( int x, int y, int w, int h, int arrows,
                 const char *dir, const char *title = "Pick a file" ) ;
  puFileSelector ( int x, int y, int w, int h,
                 const char *dir, const char *title = "Pick a file" ) ;
  puFileSelector ( int x, int y, int arrows,
                 const char* dir, const char *title = "Pick a file" ) ;
  puFileSelector ( int x, int y,
                 const char* dir, const char *title = "Pick a file" ) ;

  ~puFileSelector () ;

  /* Not for application use!! */
  puInput *__getInput () { return input ; }
  char *__getStartDir () { return startDir ; }

  void setInitialValue ( char *fname ) ;
  void setSize ( int w, int h ) ;
} ;



class puLargeInput : public puGroup
{
protected:
  int num_lines ;              // Number of lines of text in the box
  int lines_in_window ;        // Number of lines showing in the window
  int top_line_in_window ;     // Number of the first line in the window
  int max_width ;              // Width of longest line of text in box, in pixels
  int slider_width ;
  int accepting ;
  int cursor_position ;
  int select_start_position ;
  int select_end_position ;
  char *valid_data ;

  puFrame *frame ;

  puSlider *bottom_slider ;    // Horizontal slider at bottom of window
  puSlider *right_slider ;     // Vertical slider at right of window

  puArrowButton *down_arrow ;
  puArrowButton *fastdown_arrow ;
  puArrowButton *up_arrow ;
  puArrowButton *fastup_arrow ;

  char *text ;                 // Pointer to text in large input box

  short arrow_count ;          // Number of up/down arrows above and below the right slider

  int input_disabled ;

  void normalize_cursors ( void ) ;
  void removeSelectRegion ( void ) ;

public:
  puLargeInput ( int x, int y, int w, int h, int arrows, int sl_width ) ;
  ~puLargeInput () ;

  void setSize ( int w, int h ) ;

  int getNumLines () {  return num_lines ;  }
  void setTopLineInWindow ( int val ) {  top_line_in_window = val ;  }

  void draw     ( int dx, int dy ) ;
  int  checkHit ( int button, int updown, int x, int y ) ;
  void doHit    ( int button, int updown, int x, int y ) ;
  int  checkKey ( int key, int updown ) ;

  void setSelectRegion ( int s, int e ) ;
  void selectEntireLine ( void ) ;

  int  isAcceptingInput ( void ) { return accepting ; }
  void rejectInput      ( void ) { accepting = FALSE ; }

  void acceptInput ( void )
  {
    accepting = TRUE ;
    cursor_position = strlen ( getStringValue() ) ;
    select_start_position = select_end_position = -1 ;
  }

  int  getCursor ( void )  { return cursor_position ; }
  void setCursor ( int c ) { cursor_position = c ; }

  void getSelectRegion ( int *s, int *e )
  {
    if ( s ) *s = select_start_position ;
    if ( e ) *e = select_end_position   ;
  }

  char *getValidData () { return valid_data ; }
  void setValidData ( char *data )
  {
    if ( valid_data )
    {
      free ( valid_data ) ;
      valid_data = NULL ;
    }

    if ( data )
    {
      valid_data = new char[strlen(data)+1] ;
      strcpy ( valid_data, data ) ;
    }
  }

  void addValidData ( char *data )
  {
    int new_data_len = 1 ;
    if ( valid_data ) new_data_len += strlen ( valid_data ) ;
    if ( data )       new_data_len += strlen ( data ) ;
    char*new_data = new char[new_data_len] ;
    strcpy ( new_data, "\0" ) ;
    if ( valid_data ) strcat ( new_data, valid_data ) ;
    if ( data )       strcat ( new_data, data ) ;
    delete valid_data ;
    valid_data = new_data ;
  }

  int isValidCharacter ( char c )
  {
    return ( ( strchr ( valid_data, c ) != NULL ) ? 1 : 0 ) ;
  }

  void enableInput ()  {  input_disabled = FALSE ;  }
  void disableInput () {  input_disabled = TRUE ;  }
  int  inputDisabled ()  {  return input_disabled ;  }

  void  setText ( char *l ) ;
  char *getText ( void ) { return text ; }
  void  addNewLine ( char *l ) ;
  void  addText ( char *l ) ;
  void  appendText ( char *l ) ;
  void  removeText ( int start, int end ) ;
} ;

#endif
