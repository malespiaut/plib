
#include "ssgLocal.h"

void ssgLeaf::copy_from ( ssgLeaf *src, int clone_flags )
{
  ssgEntity::copy_from ( src, clone_flags ) ;

  cull_face = src -> getCullFace () ;

  ssgState *s = src -> getState () ;

  if ( s != NULL && ( clone_flags & SSG_CLONE_STATE ) )
    state = (ssgState *)( s -> clone ( clone_flags ) ) ;
  else
    state = s ;
}



ssgLeaf::ssgLeaf (void)
{
  cull_face = TRUE ;
  state = NULL ;
  type |= SSG_TYPE_LEAF ;
  dlist = 0 ;
   preDrawCB = NULL ;
  postDrawCB = NULL ;
}

ssgLeaf::~ssgLeaf (void)
{
  deleteDList () ;
}


void ssgLeaf::deleteDList ()
{
  if ( dlist != 0 )
    glDeleteLists ( dlist, 1 ) ;

  dlist = 0 ;
}


void ssgLeaf::makeDList ()
{
  deleteDList () ;  /* Just to be sure */
  dlist = glGenLists ( 1 ) ;
  glNewList ( dlist, GL_COMPILE ) ;
    draw_geometry () ;
  glEndList () ; 
}

void ssgLeaf::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  if ( isTranslucent () )
    _ssgDrawLeaf ( this ) ;
  else
    draw () ;
}

void ssgLeaf::hot ( sgVec3 s, sgMat4 m, int test_needed )
{
  int hot_result = hot_test ( s, m, test_needed ) ;

  if ( hot_result == SSG_OUTSIDE )
    return ;

  /* Add polygons to hit list! */

  hot_triangles ( s, m, hot_result != SSG_INSIDE ) ;
}



void ssgLeaf::isect ( sgSphere *s, sgMat4 m, int test_needed )
{
  int isect_result = isect_test ( s, m, test_needed ) ;

  if ( isect_result == SSG_OUTSIDE )
    return ;

  /* Add polygons to hit list! */

  isect_triangles ( s, m, isect_result != SSG_INSIDE ) ;
}


void ssgLeaf::print ( FILE *fd, char *indent )
{
  ssgEntity::print ( fd, indent ) ;

  if ( getNumParents () != getRef () )
    fprintf ( fd, "****** WARNING: Ref count doesn't equal parent count!\n" ) ;
}


int ssgLeaf::preDraw ()
{
  if ( preDrawCB != NULL && ! (*preDrawCB)(this) )
    return FALSE ;

  _ssgCurrentContext->setCullface ( getCullFace() ) ;

  return TRUE ;
}


int ssgLeaf::load ( FILE *fd )
{
  int key, t ;

  _ssgReadInt ( fd, &cull_face ) ;
  _ssgReadInt ( fd, & t ) ;

  if ( t == SSG_BACKWARDS_REFERENCE )
  {
    _ssgReadInt ( fd, & key ) ;

    if ( key == 0 )
      state = NULL ;
    else
      state = (ssgState *) _ssgGetFromList ( key ) ;
  }
  else
  if ( t == ssgTypeSimpleState() )
  {
    state = new ssgSimpleState ;

    if ( ! state -> load ( fd ) )
      return FALSE ;
  }
  else
  if ( t == ssgTypeStateSelector() )
  {
    state = new ssgStateSelector ;

    if ( ! state -> load ( fd ) )
      return FALSE ;
  }
  else
  {
    fprintf ( stderr,
       "ssgLeaf::load - Unrecognised ssgState type 0x%08x\n", t ) ;
    state = NULL ;
  }

  return ssgEntity::load(fd) ;
}


int ssgLeaf::save ( FILE *fd )
{
  _ssgWriteInt   ( fd, cull_face ) ;

  if ( state == NULL )
  {
    _ssgWriteInt ( fd, SSG_BACKWARDS_REFERENCE ) ;
    _ssgWriteInt ( fd, 0 ) ;
  }
  else
  if ( state -> getSpare () > 0 )
  {
    _ssgWriteInt ( fd, SSG_BACKWARDS_REFERENCE ) ;
    _ssgWriteInt ( fd, state -> getSpare () ) ;
  }
  else
  {
    _ssgWriteInt ( fd, state->getType() ) ;

    if ( ! state -> save ( fd ) )
      return FALSE ;
  }

  return ssgEntity::save(fd) ;
}



