
#include "ssgLocal.h"

void ssgBranch::copy_from ( ssgBranch *src, int clone_flags )
{
  ssgEntity::copy_from ( src, clone_flags ) ;

  for ( int i = 0 ; i < src -> getNumKids () ; i++ )
  {
    ssgEntity *k = src -> getKid ( i ) ;

    if ( k != NULL && ( clone_flags & SSG_CLONE_RECURSIVE ) )
      addKid ( (ssgEntity *)( k -> clone ( clone_flags )) ) ;
    else
      addKid ( k ) ;
  }
}

ssgBase *ssgBranch::clone ( int clone_flags )
{
  ssgBranch *b = new ssgBranch ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}



ssgBranch::ssgBranch (void)
{
  type |= SSG_TYPE_BRANCH ;
}


ssgBranch::~ssgBranch (void)
{
  removeAllKids () ;
}


void ssgBranch::zeroSpareRecursive ()
{
  zeroSpare () ;

  for ( ssgEntity *k = getKid ( 0 ) ; k != NULL ; k = getNextKid () )
    k -> zeroSpareRecursive () ;
}


void ssgBranch::recalcBSphere (void)
{
  emptyBSphere () ;

  for ( ssgEntity *k = getKid ( 0 ) ; k != NULL ; k = getNextKid () )
    extendBSphere ( k -> getBSphere () ) ;

  bsphere_is_invalid = FALSE ;
}


void ssgBranch::addKid ( ssgEntity *entity )
{
  kids. addEntity ( entity ) ;
  entity -> addParent ( this ) ;
  dirtyBSphere () ;
}


void ssgBranch::removeKid  ( int n )
{
  ssgEntity *k = kids.getEntity ( n ) ;

  if ( k != NULL )
    removeKid ( k ) ;
}


void ssgBranch::removeKid ( ssgEntity *entity )
{
  entity -> removeParent ( this ) ;
  kids.removeEntity ( entity ) ;
  dirtyBSphere () ;
}


void ssgBranch::removeAllKids (void)
{
  ssgEntity *k ;

  while ( ( k = getKid ( 0 ) ) != NULL )
    removeKid ( k ) ;
}


void ssgBranch::replaceKid ( int n, ssgEntity *new_entity )
{
  if ( n >= 0 && n < getNumKids () )
  {
    getKid ( n ) -> removeParent ( this ) ;
    kids.replaceEntity ( n, new_entity ) ;
    new_entity -> addParent ( this ) ;
    dirtyBSphere () ;
  }
}

void ssgBranch::replaceKid ( ssgEntity *old_entity, ssgEntity *new_entity )
{
  replaceKid ( searchForKid( old_entity ), new_entity ) ;
}


void ssgBranch::print ( FILE *fd, char *indent, int how_much )
{
  ssgEntity::print ( fd, indent, how_much ) ;
  fprintf ( fd, "%s  Num Kids=%d\n", indent, getNumKids() ) ;

  if ( getNumParents() != getRef() )
    ulSetError ( UL_WARNING, "Ref count doesn't tally with parent count" ) ;

	if ( how_much > 1 )
  {	if ( bsphere.isEmpty() )
			fprintf ( fd, "%s  BSphere is Empty.\n", indent ) ;
		else
			fprintf ( fd, "%s  BSphere  R=%g, C=(%g,%g,%g)\n", indent,
				bsphere.getRadius(), bsphere.getCenter()[0], bsphere.getCenter()[1], bsphere.getCenter()[2] ) ;
	}

  char in [ 100 ] ;
  sprintf ( in, "%s  ", indent ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> print ( fd, in, how_much ) ;
}


ssgEntity *ssgBranch::getByName ( char *match )
{
  if ( getName() != NULL && strcmp ( getName(), match ) == 0 )
    return this ;

  /* Otherwise check the kids for a match */

  for ( ssgEntity* k = getKid ( 0 ) ; k != NULL ; k = getNextKid() )
  {
    ssgEntity *e = k -> getByName ( match ) ;

    if ( e != NULL )
      return e ;
  }

  return NULL ;
}


ssgEntity *ssgBranch::getByPath ( char *path )
{
  /* Ignore leading '/' */

  if ( *path == '/' )
    ++path ;

  char *n = getName () ;

  /*
    If this node has no name then pass the request down the tree
  */

  if ( n == NULL )
  {
    for ( ssgEntity* k = getKid ( 0 ) ; k != NULL ; k = getNextKid () )
    {
      ssgEntity *e = k -> getByPath ( path ) ;

      if ( e != NULL )
        return e ;
    }

    return NULL ;
  }

  /*
    If this node does have a name - but it doesn't match the
    next part of the path then punt.
  */

  unsigned int l = strlen ( n ) ;

  if ( strlen ( path ) < l || strncmp ( n, path, l ) != 0 )
    return NULL ;

  /*
    If the first part of the path is this ssgBranch, we
    may have a winner.
  */

  char c = path [ l ] ;

  /* If we reached the end of the path - we win! */

  if ( c == '\0' )
    return this ;

  if ( c == '/' )
  {
    /* If the path continues, try to follow the path to the kids */

    for ( ssgEntity* k = getKid ( 0 ) ; k != NULL ; k = getNextKid () )
    {
      ssgEntity *e = k -> getByPath ( path + l ) ;

      if ( e != NULL )
        return e ;
    }
  }

  return NULL ;
}
 


void ssgBranch::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_CULL ) )
    return ;

  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> cull ( f, m, cull_result != SSG_INSIDE ) ;

  postTravTests ( SSGTRAV_CULL ) ; 
}



void ssgBranch::hot ( sgVec3 s, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_HOT ) )
    return ;

  int hot_result = hot_test ( s, m, test_needed ) ;

  if ( hot_result == SSG_OUTSIDE )
    return ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> hot ( s, m, hot_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_HOT ) ; 
}



void ssgBranch::isect ( sgSphere *s, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_ISECT ) )
    return ;

  int isect_result = isect_test ( s, m, test_needed ) ;

  if ( isect_result == SSG_OUTSIDE )
    return ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> isect ( s, m, isect_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_ISECT ) ; 
}


int ssgBranch::load ( FILE *fd )
{
  int nkids ;

  _ssgReadInt ( fd, & nkids ) ;

  if ( ! ssgEntity::load ( fd ) )
    return FALSE ;

  for ( int i = 0 ; i < nkids ; i++ )
  {
    int key, t ;
    ssgEntity *kid ;

    _ssgReadInt ( fd, & t ) ;

    if ( t == SSG_BACKWARDS_REFERENCE )
    {
      _ssgReadInt ( fd, & key ) ;
      kid = (ssgEntity *) _ssgGetFromList ( key ) ; 
    }
    else
    if ( t == ssgTypeVTable       () ) kid = new ssgVTable       () ; else
    if ( t == ssgTypeVtxTable     () ) kid = new ssgVtxTable     () ; else
    if ( t == ssgTypeVtxArray     () ) kid = new ssgVtxArray     () ; else
    if ( t == ssgTypeBranch       () ) kid = new ssgBranch       () ; else
    if ( t == ssgTypeTransform    () ) kid = new ssgTransform    () ; else
    if ( t == ssgTypeTexTrans     () ) kid = new ssgTexTrans     () ; else
    if ( t == ssgTypeSelector     () ) kid = new ssgSelector     () ; else
    if ( t == ssgTypeRangeSelector() ) kid = new ssgRangeSelector() ; else
    if ( t == ssgTypeTimedSelector() ) kid = new ssgTimedSelector() ; else
    if ( t == ssgTypeCutout       () ) kid = new ssgCutout       () ; else
    if ( t == ssgTypeInvisible    () ) kid = new ssgInvisible    () ; else
    if ( t == ssgTypeRoot         () ) kid = new ssgRoot         () ; else
    {
      ulSetError ( UL_WARNING, "loadSSG: Unrecognised Entity type 0x%08x", t ) ;
      return FALSE ;
    }

    if ( ! kid -> load ( fd ) )
    {
      ulSetError ( UL_WARNING, "loadSSG: Failed to read child object." ) ;
      return FALSE ;
    }

    kid -> recalcBSphere () ;
    addKid ( kid ) ;
  }

  return TRUE ;
}


int ssgBranch::save ( FILE *fd )
{
  _ssgWriteInt ( fd, getNumKids() ) ;

  if ( ! ssgEntity::save ( fd ) )
    return FALSE ;

  for ( int i = 0 ; i < getNumKids() ; i++ )
  {
    ssgEntity *kid = getKid ( i ) ;

    /* Has this child node already been written out? */

 
#ifdef WRITE_SSG_VERSION_ZERO
    // Pfusch, kludge, fix me: use index array
		if(kid ->isAKindOf(ssgTypeVtxArray()))
		{
      _ssgWriteInt ( fd, ssgTypeVtxTable() ) ;

			ssgVtxArray *svt=(ssgVtxArray *)kid;
			if ( ! svt -> ssgVtxTable::save ( fd ) )
			{
				ulSetError ( UL_WARNING, "saveSSG: Failed to write child object" ) ;
				return FALSE ;
			}
		}
		else if(kid ->isAKindOf(ssgTypeSelector()))
		{
      _ssgWriteInt ( fd, ssgTypeBranch() ) ;

			ssgSelector *sel=(ssgSelector *)kid;
			if ( ! sel -> ssgBranch::save ( fd ) )
			{
				ulSetError ( UL_WARNING, "saveSSG: Failed to write child object" ) ;
				return FALSE ;
			}
		}
		else
		{ if ( (unsigned long)kid->getType() < (unsigned long)0x01000000 ) // don't save ssgAux stuff
			{
				_ssgWriteInt ( fd, kid->getType() ) ;

				if ( ! kid -> save ( fd ) )
				{
					ulSetError ( UL_WARNING, "saveSSG: Failed to write child object" ) ;
					return FALSE ;
				}
			}
		}
#else
    if ( kid -> getSpare () > 0 )
    {
      _ssgWriteInt ( fd, SSG_BACKWARDS_REFERENCE ) ;
      _ssgWriteInt ( fd, kid -> getSpare () ) ;
    }
    else
    { _ssgWriteInt ( fd, kid->getType() ) ;

			if ( ! kid -> save ( fd ) )
			{
				ulSetError ( UL_WARNING, "saveSSG: Failed to write child object" ) ;
				return FALSE ;
			}
    }
#endif
  }

  return TRUE ;
}


