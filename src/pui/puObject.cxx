
#include "puLocal.h"

inline float clamp01 ( float x )
{
  return (x >= 1.0f) ? 1.0f : x ;
}

static void load_colour_scheme ( float col[][4], float r, float g,
                                                 float b, float a )
{
  puSetColour ( col [ PUCOL_FOREGROUND ], r, g, b, a ) ;
  puSetColour ( col [ PUCOL_BACKGROUND ], r/2.0f, g/2.0f, b/2.0f, a ) ;
  puSetColour ( col [ PUCOL_HIGHLIGHT  ], clamp01(r*1.3f), clamp01(g*1.3f),
                                             clamp01(b*1.3f), a ) ;

  if ( 4.0f * g + 3.0f * r + b > 4.0f )
    puSetColour ( col [ PUCOL_LEGEND ], 0.0f, 0.0f, 0.0f, a ) ;
  else
    puSetColour ( col [ PUCOL_LEGEND ], 1.0f, 1.0f, 1.0f, a ) ;
}


static int    defaultStyle = PUSTYLE_DEFAULT ;
static puFont defaultLegendFont ;
static puFont defaultLabelFont  ;
static float  defaultColourScheme [ 4 ] ;

void puSetDefaultStyle ( int  style ) { defaultStyle = style ; }
int  puGetDefaultStyle ( void ) { return defaultStyle ; }

void puSetDefaultFonts ( puFont legendFont, puFont labelFont )
{
  defaultLegendFont = legendFont ;
  defaultLabelFont  = labelFont  ;
}

puFont puGetDefaultLabelFont  () { return defaultLabelFont  ; }
puFont puGetDefaultLegendFont () { return defaultLegendFont ; }

void puGetDefaultFonts ( puFont *legendFont, puFont *labelFont )
{
  if ( legendFont ) *legendFont = defaultLegendFont ;
  if ( labelFont  ) *labelFont  = defaultLabelFont  ;
}

void puSetDefaultColourScheme ( float r, float g, float b, float a )
{
  defaultColourScheme[0] = r ;
  defaultColourScheme[1] = g ;
  defaultColourScheme[2] = b ;
  defaultColourScheme[3] = a ;
  load_colour_scheme ( _puDefaultColourTable, r, g, b, a ) ;
}

void puGetDefaultColourScheme ( float *r, float *g, float *b, float *a )
{
  if ( r ) *r = defaultColourScheme[0] ;
  if ( g ) *g = defaultColourScheme[1] ;
  if ( b ) *b = defaultColourScheme[2] ;
  if ( a ) *a = defaultColourScheme[3] ;
}



void puObject::setColourScheme ( float r, float g, float b, float a )
{
  load_colour_scheme ( colour, r, g, b, a ) ;
}

puObject::puObject ( int minx, int miny, int maxx, int maxy ) : puValue ()
{
  type |= PUCLASS_OBJECT ;
  bbox.min[0] = abox.min[0] = minx ;
  bbox.min[1] = abox.min[1] = miny ;
  bbox.max[0] = abox.max[0] = maxx ;
  bbox.max[1] = abox.max[1] = maxy ;

  active_mouse_edge = PU_UP ;
  style      = defaultStyle ;
  visible = active  = TRUE  ;
  highlighted       = FALSE ;
  am_default        = FALSE ;
  window            = puGetWindow () ;

  cb          = NULL ;
  active_cb   = NULL ;
  down_cb     = NULL ;
  r_cb        = NULL ;
  render_data = NULL ;
  user_data   = NULL ;
  next = prev = NULL ;
  label       = NULL ;
  labelPlace  = PUPLACE_DEFAULT   ;
  labelFont   = defaultLabelFont  ;
  legend      = NULL ;
  legendFont  = defaultLegendFont ;

  for ( int i = 0 ; i < PUCOL_MAX ; i++ )
    puSetColour ( colour[i], _puDefaultColourTable[i] ) ;

  if ( ! puNoGroup() )
  {
    parent = puGetCurrGroup() ;
    parent -> add ( this ) ;
  }
  else
    parent = NULL ;
}


puObject::~puObject ()
{
  if ( parent != this && parent != NULL )
    parent -> remove ( this ) ;

  if ( this == puActiveWidget () )
    puDeactivateWidget () ;
}

void puObject::recalc_bbox ( void )
{
  bbox = abox ;

  if ( label != NULL )
    switch ( labelPlace )
    {
      case PUPLACE_ABOVE : bbox.max[1] += 
              puGetStringHeight    ( getLabelFont (), getLabel () ) +
              puGetStringDescender ( getLabelFont () ) +
              PUSTR_TGAP + PUSTR_BGAP ; break ;
      case PUPLACE_BELOW : bbox.min[1] -=
              puGetStringHeight ( getLabelFont() ) +
              puGetStringDescender ( getLabelFont () ) +
              PUSTR_TGAP + PUSTR_BGAP ; break ;
      case PUPLACE_LEFT  : bbox.min[0] -=
              puGetStringWidth  ( getLabelFont(), getLabel() ) +
              PUSTR_LGAP + PUSTR_RGAP ; break ;
      case PUPLACE_RIGHT : bbox.max[0] +=
              puGetStringWidth  ( getLabelFont(), getLabel() ) +
              PUSTR_LGAP + PUSTR_RGAP ; break ;
    }

  if ( parent != NULL )
    parent -> recalc_bbox () ;
}

void puObject::draw_label ( int dx, int dy )
{
  if ( !visible ) return ;

  /* If greyed out then halve the opacity when drawing the label */

  if ( active )
    glColor4fv ( colour [ PUCOL_LABEL ] ) ;
  else
    glColor4f ( colour [ PUCOL_LABEL ][0],
                colour [ PUCOL_LABEL ][1],
                colour [ PUCOL_LABEL ][2],
                colour [ PUCOL_LABEL ][3] / 2.0f ) ; /* 50% more transparent */

  switch ( labelPlace )
  {
    case PUPLACE_ABOVE : puDrawString ( labelFont, label, dx + abox.min[0] + PUSTR_LGAP, dy + abox.max[1] + puGetStringDescender(labelFont) + PUSTR_BGAP ) ; break ;
    case PUPLACE_BELOW : puDrawString ( labelFont, label, dx + abox.min[0] + PUSTR_LGAP, dy + bbox.min[1] + puGetStringDescender(labelFont) + PUSTR_BGAP ) ; break ;
    case PUPLACE_LEFT  : puDrawString ( labelFont, label, dx + bbox.min[0] + PUSTR_LGAP, dy + abox.min[1] + puGetStringDescender(labelFont) + PUSTR_BGAP ) ; break ;
    case PUPLACE_RIGHT : puDrawString ( labelFont, label, dx + abox.max[0] + PUSTR_LGAP, dy + abox.min[1] + puGetStringDescender(labelFont) + PUSTR_BGAP ) ; break ;
  }
}


int puObject::checkKey ( int key, int updown )
{
  if ( updown == PU_UP )
    return FALSE ;

  if ( isReturnDefault() && ( key == '\r' || key == '\n' ) && ( window == puGetWindow () ) )
  {
    if ( puActiveWidget() && ( this != puActiveWidget() ) )
    {
      puActiveWidget() -> invokeDownCallback () ;
      puDeactivateWidget () ;
    }

    checkHit ( PU_LEFT_BUTTON, PU_DOWN, (abox.min[0]+abox.max[0])/2,
                                        (abox.min[1]+abox.max[1])/2 ) ;
    checkHit ( PU_LEFT_BUTTON, PU_UP  , (abox.min[0]+abox.max[0])/2,
                                        (abox.min[1]+abox.max[1])/2 ) ;
    return TRUE ;
  }

  return FALSE ;
}


void puObject::doHit ( int button, int updown, int /* x */, int /* y */ )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( button == PU_LEFT_BUTTON )
  {
    if ( updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN )
    {
      lowlight () ;
      puSetActiveWidget ( this ) ;
      invokeCallback () ;
    }
    else
      highlight () ;
  }
  else
    lowlight () ;
}

int puObject::checkHit ( int button, int updown, int x, int y )
{
  if ( isHit( x, y ) )
  {
    doHit ( button, updown, x, y ) ;
    return TRUE ;
  }

  lowlight () ;
  return FALSE ;
}

 
char *puValue::getTypeString ( void )
{
  int i = getType () ;

  if ( i & PUCLASS_DIALOGBOX   ) return "puDialogBox" ;
  if ( i & PUCLASS_SLIDER      ) return "puSlider" ;
  if ( i & PUCLASS_BUTTONBOX   ) return "puButtonBox" ;
  if ( i & PUCLASS_INPUT       ) return "puInput" ;
  if ( i & PUCLASS_MENUBAR     ) return "puMenuBar" ;
  if ( i & PUCLASS_POPUPMENU   ) return "puPopupMenu" ;
  if ( i & PUCLASS_POPUP       ) return "puPopup" ;
  if ( i & PUCLASS_ONESHOT     ) return "puOneShot" ;
  if ( i & PUCLASS_BUTTON      ) return "puButton" ;
  if ( i & PUCLASS_TEXT        ) return "puText" ;
  if ( i & PUCLASS_FRAME       ) return "puFrame" ;
  if ( i & PUCLASS_GROUP       ) return "puGroup" ;
  if ( i & PUCLASS_INTERFACE   ) return "puInterface" ;
  if ( i & PUCLASS_OBJECT      ) return "puObject" ;
  if ( i & PUCLASS_VALUE       ) return "puValue" ;

  return "Unknown Object type." ;
}


