
#include "ssgLocal.h"


#ifdef _SSG_USE_DLIST

enum _ssgDListType
{
  SSG_DLIST_LEAF,
  SSG_DLIST_LOAD_MATRIX,
  SSG_DLIST_PUSH_MATRIX,
  SSG_DLIST_POP_MATRIX,
  SSG_DLIST_LOAD_TEX_MATRIX,
  SSG_DLIST_UNLOAD_TEX_MATRIX,
  SSG_DLIST_NOTHING
} ;

class _ssgDList
{
public:
  _ssgDListType type ;
  sgMat4        mat  ;
  ssgLeaf      *leaf ;

  _ssgDList () { setEmpty () ; }

  void setPopMatrix ()
  {
    type = SSG_DLIST_POP_MATRIX ;
  }

  void setLoadTexMatrix ( sgMat4  m )
  {
    sgCopyMat4 ( mat, m ) ;
    type = SSG_DLIST_LOAD_TEX_MATRIX ;
  }

  void setUnloadTexMatrix ()
  {
    type = SSG_DLIST_UNLOAD_TEX_MATRIX ;
  }

  void setLoadMatrix ( sgMat4  m )
  {
    sgCopyMat4 ( mat, m ) ;
    type = SSG_DLIST_LOAD_MATRIX ;
  }

  void setPushMatrix ( sgMat4  m )
  {
    sgCopyMat4 ( mat, m ) ;
    type = SSG_DLIST_PUSH_MATRIX ;
  }

  void setDrawLeaf ( ssgLeaf *l )
  {
    leaf = l ;
    type = SSG_DLIST_LEAF ;
  }

  void setEmpty ()
  {
    type = SSG_DLIST_NOTHING ;
  }

  void draw ()
  {
    switch ( type )
    {
      case SSG_DLIST_LEAF :
        leaf -> draw () ;
        break ;

      case SSG_DLIST_POP_MATRIX :
        glPopMatrix () ;
        break ;

      case SSG_DLIST_LOAD_TEX_MATRIX :
        glMatrixMode  ( GL_TEXTURE ) ;
        glLoadMatrixf ( (float *) mat ) ;
        glMatrixMode  ( GL_MODELVIEW ) ;
        break ;

      case SSG_DLIST_UNLOAD_TEX_MATRIX :
        glMatrixMode   ( GL_TEXTURE ) ;
        glLoadIdentity () ;
        glMatrixMode   ( GL_MODELVIEW ) ;
        break ;

      case SSG_DLIST_LOAD_MATRIX :
        glLoadMatrixf ( (float *) mat ) ;
        break ;

      case SSG_DLIST_PUSH_MATRIX :
        glPushMatrix () ;
        glLoadMatrixf ( (float *) mat ) ;
        break ;

      default: break ;
    }

    setEmpty () ;
  }
} ;


#define MAX_DLIST   2048

static int next_dlist = 0 ;
static _ssgDList dlist [ MAX_DLIST ] ;

void _ssgDrawDList ()
{
  for ( int i = 0 ; i < next_dlist ; i++ )
    dlist [ i ] . draw () ;

  next_dlist = 0 ;
}

void _ssgPushMatrix ( sgMat4 m )
{
  /*
    There is no point in having a  pop/push sequence,
    so optimise it to a simple load.
  */

  if ( next_dlist > 0 &&
       dlist [ next_dlist - 1 ] . type == SSG_DLIST_POP_MATRIX )
  {
    next_dlist-- ;
    _ssgLoadMatrix ( m ) ;
  }
  else
  if ( next_dlist >= MAX_DLIST )
    ulSetError ( UL_WARNING, "DList stack overflow!" ) ;
  else
    dlist [ next_dlist++ ] . setPushMatrix ( m ) ;
}


void _ssgPopMatrix ()
{
  /*
    There is no point in having a  push/pop sequence,
    so optimise it to a single load.
  */

  if ( next_dlist > 0 &&
       dlist [ next_dlist - 1 ] . type == SSG_DLIST_PUSH_MATRIX )
    next_dlist-- ;
  else
  if ( next_dlist >= MAX_DLIST )
    ulSetError ( UL_WARNING, "DList stack overflow!" ) ;
  else
    dlist [ next_dlist++ ] . setPopMatrix () ;
}


void _ssgLoadTexMatrix ( sgMat4 m )
{
  /*
    There is no point in having a unload/load or a load/load sequence,
    so optimise it to a single load.
  */

  while ( next_dlist > 0 &&
          ( dlist [ next_dlist - 1 ] . type == SSG_DLIST_LOAD_TEX_MATRIX ||
            dlist [ next_dlist - 1 ] . type == SSG_DLIST_UNLOAD_TEX_MATRIX )
        )
    next_dlist-- ;

  if ( next_dlist >= MAX_DLIST )
    ulSetError ( UL_WARNING, "DList stack overflow!" ) ;
  else
    dlist [ next_dlist++ ] . setLoadTexMatrix ( m ) ;
}



void _ssgUnloadTexMatrix ()
{
  /*
    There is no point in having a unload/unload or a load/unload sequence,
    so optimise it to a single unload.
  */

  while ( next_dlist > 0 &&
          ( dlist [ next_dlist - 1 ] . type == SSG_DLIST_LOAD_TEX_MATRIX ||
            dlist [ next_dlist - 1 ] . type == SSG_DLIST_UNLOAD_TEX_MATRIX )
        )
    next_dlist-- ;

  if ( next_dlist >= MAX_DLIST )
    ulSetError ( UL_WARNING, "DList stack overflow!" ) ;
  else
    dlist [ next_dlist++ ] . setUnloadTexMatrix () ;
}



void _ssgLoadMatrix ( sgMat4 m )
{
  /*
    There is no point in having a  load/load sequence,
    so optimise it to a single load.
  */

  while ( next_dlist > 0 &&
          dlist [ next_dlist - 1 ] . type == SSG_DLIST_LOAD_MATRIX )
    next_dlist-- ;

  if ( next_dlist >= MAX_DLIST )
    ulSetError ( UL_WARNING, "DList stack overflow!" ) ;
  else
    dlist [ next_dlist++ ] . setLoadMatrix ( m ) ;
}



void _ssgDrawLeaf ( ssgLeaf *l )
{
  if ( next_dlist >= MAX_DLIST )
    ulSetError ( UL_WARNING, "DList stack overflow!" ) ;
  else
    dlist [ next_dlist++ ] . setDrawLeaf ( l ) ;
}

#endif // #ifdef _SSG_USE_DLIST
