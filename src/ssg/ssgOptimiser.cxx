
#include "ssgLocal.h"

static float optimise_vtol [3] =
{
  0.01f,   /* DISTANCE_SLOP = One centimeter */
  0.04f,   /* COLOUR_SLOP = Four percent */
  0.004f,  /* TEXCOORD_SLOP = One texel on a 256 map */
} ;

static float* current_vtol = 0 ;

#define DISTANCE_SLOP   current_vtol[0]
#define COLOUR_SLOP     current_vtol[1]
#define TEXCOORD_SLOP   current_vtol[2]

inline float frac ( float x )
{
  return x - (float) floor(x) ;
}

struct OptVertex
{
  sgVec3 vertex ;
  sgVec3 normal ;
  sgVec2 texcoord ;
  sgVec4 colour ;
  int    counter ;
  
  void print () { ulSetError ( UL_DEBUG, "%d:(%g,%g,%g):(%g,%g):(%g,%g,%g,%g):(%g,%g,%g)",
    counter,
    vertex[0],vertex[1],vertex[2],
    texcoord[0],texcoord[1],
    colour[0],colour[1],colour[2],colour[3],
    normal[0],normal[1],normal[2] ) ; }
  
  OptVertex ( sgVec3 v, sgVec2 t, sgVec4 c )
  {
    sgCopyVec3 ( vertex  , v ) ;
    sgCopyVec2 ( texcoord, t ) ;
    sgCopyVec4 ( colour  , c ) ;
    sgSetVec3  ( normal  , 0.0f, 0.0f, 0.0f ) ;
    counter = 1 ;
  }
  
  int equal ( sgVec3 v, sgVec2 t, sgVec4 c, int tex_frac )
  {
    if ( ! sgCompareVec3 ( vertex  , v, DISTANCE_SLOP ) == 0 ||
      ! sgCompareVec4 ( colour  , c, COLOUR_SLOP   ) == 0 )
      return FALSE ;
    
    if ( ! tex_frac )
      return sgCompareVec2 ( texcoord, t, TEXCOORD_SLOP ) == 0 ;
    
    return fabs ( frac ( texcoord[0] ) - frac ( t[0] ) ) <= TEXCOORD_SLOP &&
      fabs ( frac ( texcoord[1] ) - frac ( t[1] ) ) <= TEXCOORD_SLOP ;
  }
  
  void bump () { counter++ ; }
  void dent () { counter-- ; }
  int getCount () { return counter ; }
} ;


#define MAX_OPT_VERTEX_LIST 10000

class OptVertexList
{
public:
  short vnum, tnum ;
  OptVertex **vlist ;
  short      *tlist ;
  ssgState  *state ;
  int        cullface ;
  
  OptVertexList ( ssgState *s, int cf )
  {
    /*
    Have to dynamically allocate these to get
    around Mac's CodeWarrior restriction on
    32Kb as max structure size.
    */
    
    vlist = new OptVertex* [ MAX_OPT_VERTEX_LIST ] ;
    tlist = new short [ MAX_OPT_VERTEX_LIST * 3 ] ;
    state = s ;
    cullface = cf ;
    vnum = tnum = 0 ;
  }
  
  ~OptVertexList ()
  {
    for ( int i = 0 ; i < vnum ; i++ )
      delete vlist [ i ] ;
    
    delete vlist ;
    delete tlist ;
  }
  
  short find ( sgVec3 v, sgVec2 t, sgVec4 c, int tex_fraction_only = FALSE ) ;
  
  short add ( sgVec3 v1, sgVec2 t1, sgVec4 c1,
    sgVec3 v2, sgVec2 t2, sgVec4 c2,
    sgVec3 v3, sgVec2 t3, sgVec4 c3 ) ;
  short add ( sgVec3 v, sgVec2 t, sgVec4 c ) ;
  short add ( short v1, short v2, short v3 ) ;
  void  add ( ssgLeaf *l ) ;
  
  void makeNormals () ;
  
  void print ()
  {
    ulSetError ( UL_DEBUG, "LIST: %d unique vertices and %d triangles",
      vnum, tnum ) ;
  }
  
  void follow ( int tri, int v1, int v2, int backwards, int *len, short *list, short *next ) ;

  int getLeastConnected ( short *t, short *v )
  {
    int least = 32767 ;
    *v = 0 ;

    /* Find the least connected vertex that *is* connected */

    int i ;

    for ( i = 0 ; i < vnum ; i++ )
    {
      int c = vlist [ i ] -> getCount () ;

      if ( c > 0 && c < least )
      {
        least = c ;
        *v = i ;
      }
    }

    if ( least == 32767 )  /* Didn't find an unused vertex - so punt. */
      return FALSE ;

    least = 32767 ;
    *t = 32767 ;

    for ( i = 0 ; i < tnum ; i++ )
      if ( tlist[i*3+0] == *v || tlist[i*3+1] == *v || tlist[i*3+2] == *v )
      {
        int c = vlist [ tlist[i*3+0] ] -> getCount () +
                vlist [ tlist[i*3+1] ] -> getCount () +
                vlist [ tlist[i*3+2] ] -> getCount () ;

        if ( c < least )
        {
          least = c ;
          *t = i ;
        }
      }

    if ( least == 32767 )  /* Didn't find an unused vertex - so punt. */
      return FALSE ;

    return TRUE ;  /* Got it! */
  }

  void sub ( short t )
  {
    vlist [ tlist[t*3+0] ] -> dent () ;
    vlist [ tlist[t*3+1] ] -> dent () ;
    vlist [ tlist[t*3+2] ] -> dent () ;
    
    tlist[t*3+0] = -1 ;
    tlist[t*3+1] = -1 ;
    tlist[t*3+2] = -1 ;
  }
} ;


short OptVertexList::add ( sgVec3 v1, sgVec2 t1, sgVec4 c1,
                          sgVec3 v2, sgVec2 t2, sgVec4 c2,
                          sgVec3 v3, sgVec2 t3, sgVec4 c3 )
{
  /*
  Sharing vertices is tricky because of texture coordinates
  that have the same all-important fractional part - but
  differ in their integer parts.
  */
  
  sgVec2 adjust ;
  
  /* Find which (if any) of the vertices are a match for one in the list */
  
  short vi1 = find ( v1, t1, c1, TRUE ) ;
  short vi2 = find ( v2, t2, c2, TRUE ) ;
  short vi3 = find ( v3, t3, c3, TRUE ) ;
  
  /* Compute texture offset coordinates (if needed) to make everything match */
  
  if ( vi1 >= 0 )
    sgSubVec2 ( adjust, t1, vlist[vi1]->texcoord ) ;
  else
    if ( vi2 >= 0 )
      sgSubVec2 ( adjust, t2, vlist[vi2]->texcoord ) ;
    else
      if ( vi3 >= 0 )
        sgSubVec2 ( adjust, t3, vlist[vi3]->texcoord ) ;
      else
      {
      /*
      OK, there was no match - so just remove
      any large numbers from the texture coords
        */
        
        adjust [ 0 ] = (float) floor ( t1[0] ) ;
        adjust [ 1 ] = (float) floor ( t1[1] ) ;
      }
      
      /*
      Now adjust the texture coordinates and add them into the list
      */
      sgVec2 tmp ;
      sgSubVec2 ( tmp, t1, adjust ) ; vi1 = add ( v1, tmp, c1 ) ;
      sgSubVec2 ( tmp, t2, adjust ) ; vi2 = add ( v2, tmp, c2 ) ;
      sgSubVec2 ( tmp, t3, adjust ) ; vi3 = add ( v3, tmp, c3 ) ;
      
      return add ( vi1, vi2, vi3 ) ;
}


short OptVertexList::add ( short v1, short v2, short v3 )
{
  if ( v1 == v2 || v2 == v3 || v3 == v1 ) /* Toss degenerate triangles */
  {
    vlist [ v1 ] -> dent () ; /* Un-reference their vertices */
    vlist [ v2 ] -> dent () ;
    vlist [ v3 ] -> dent () ;
    return -1 ;
  }
  
  tlist [ tnum*3+ 0 ] = v1 ;
  tlist [ tnum*3+ 1 ] = v2 ;
  tlist [ tnum*3+ 2 ] = v3 ;
  
  return tnum++ ;
}


void OptVertexList::makeNormals()
{
  short i ;
  
  for ( i = 0 ; i < vnum ; i++ )
    sgSetVec3 ( vlist [ i ] -> normal, 0.0f, 0.0f, 0.0f ) ;
  
  for ( i = 0 ; i < tnum ; i++ )
  {
    sgVec3 tmp ;
    
    sgMakeNormal ( tmp, vlist [ tlist [ i*3+ 0 ] ] -> vertex,
      vlist [ tlist [ i*3+ 1 ] ] -> vertex,
      vlist [ tlist [ i*3+ 2 ] ] -> vertex ) ;
    
    sgAddVec3 ( vlist [ tlist [ i*3+ 0 ] ] -> normal, tmp ) ;
    sgAddVec3 ( vlist [ tlist [ i*3+ 1 ] ] -> normal, tmp ) ;
    sgAddVec3 ( vlist [ tlist [ i*3+ 2 ] ] -> normal, tmp ) ;
  }
  
  for ( i = 0 ; i < vnum ; i++ )
    if ( sgScalarProductVec2 ( vlist[i]->normal, vlist[i]->normal ) < 0.001 )
      sgSetVec3 ( vlist[i]->normal, 0.0f, 0.0f, 1.0f ) ;
    else
      sgNormaliseVec3 ( vlist [ i ] -> normal ) ;
}

short OptVertexList::find ( sgVec3 v, sgVec2 t, sgVec4 c, int tex_frac )
{
  for ( short i = 0 ; i < vnum ; i++ )
  {
    if ( vlist[i] -> equal ( v, t, c, tex_frac ) )
      return i ;
  }
  return -1 ;
}

short OptVertexList::add ( sgVec3 v, sgVec2 t, sgVec4 c )
{
  short i = find ( v, t, c, FALSE ) ;
  
  if ( i >= 0 )
  {
    vlist [ i ] -> bump () ;
    return i ;
  }
  
  vlist [ vnum ] = new OptVertex ( v, t, c ) ;
  return vnum++ ;
} 

void OptVertexList::add ( ssgLeaf *l )
{
  int j ;

  for ( j = 0 ; j < l -> getNumTriangles () ; j ++ )
  {
    short v1, v2, v3 ;
    l -> getTriangle ( j, &v1, &v2, &v3 ) ;
    add ( l->getVertex ( v1 ), l->getTexCoord ( v1 ), l->getColour ( v1 ),
      l->getVertex ( v2 ), l->getTexCoord ( v2 ), l->getColour ( v2 ),
      l->getVertex ( v3 ), l->getTexCoord ( v3 ), l->getColour ( v3 ) ) ;
  }
}


void OptVertexList::follow ( int tri, int v1, int v2, int backwards, int *len,
                                             short *new_vlist, short *new_vc )
{
  /*  WARNING  -  RECURSIVE !!  */

  v1 = tlist [ tri*3+ v1 ] ;
  v2 = tlist [ tri*3+ v2 ] ;

  /*
    This triangle's work is done - dump it.
  */

  (*len)++ ;
  sub ( tri ) ;

  /*
    If the exit edge vertices don't *both* have a reference
    then we are done.
  */

  if ( vlist [ v1 ] -> getCount () <= 0 ||
       vlist [ v2 ] -> getCount () <= 0 )
    return ;

  /*
    Search for a polygon that shares that edge in the correct
    direction - and follow it.
  */

  for ( int i = 0 ; i < tnum ; i++ )
  {
    if ( tlist [ i*3+ 0 ] < 0 )  /* Deleted triangle */
      continue ;

    if ( backwards )
    {
      /* If the previous polygon was backwards */

      if ( tlist [ i*3+ 0 ] == v1 && tlist [ i*3+ 2 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 1 ] ;
	follow ( i, 0, 1, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
      else
      if ( tlist [ i*3+ 1 ] == v1 && tlist [ i*3+ 0 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 2 ] ;
	follow ( i, 1, 2, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
      else
      if ( tlist [ i*3+ 2 ] == v1 && tlist [ i*3+ 1 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 0 ] ;
	follow ( i, 2, 0, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
    }
    else
    {
      /* If the previous polygon was forwards... */

      if ( tlist [ i*3+ 0 ] == v1 && tlist [ i*3+ 2 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 1 ] ;
	follow ( i, 1, 2, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
      else
      if ( tlist [ i*3+ 1 ] == v1 && tlist [ i*3+ 0 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 2 ] ;
	follow ( i, 2, 0, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
      else
      if ( tlist [ i*3+ 2 ] == v1 && tlist [ i*3+ 1 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 0 ] ;
	follow ( i, 0, 1, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
    }
  }
}


static ssgLeaf** build_leaf_list ( ssgEntity *ent, ssgLeaf** leaf_list=0 )
{
  enum { MAX_LEAF_COUNT = 10000 } ;
  static int leaf_count ;

  if ( leaf_list == NULL )
  {
    leaf_list = new ssgLeaf* [ MAX_LEAF_COUNT+1 ] ;
    leaf_count = 0 ;
    leaf_list [ leaf_count ] = NULL ;
  }

  if ( ent -> isAKindOf ( ssgTypeBranch () ) )
  {
    ssgBranch *b_ent = (ssgBranch *) ent ;
    for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
      k = b_ent -> getNextKid () )
    {
      build_leaf_list ( k, leaf_list ) ;
    }
  }
  else if ( ent -> isAKindOf ( ssgTypeLeaf () ) )
  {
    ssgLeaf  *l = (ssgLeaf *) ent ;

    bool found = false ;
    for ( int i = 0 ; leaf_list [ i ] != NULL ; i ++ )
    {
      if ( leaf_list [ i ] == l )
      {
        found = true ;
        break ;
      }
    }

    if ( !found && leaf_count < MAX_LEAF_COUNT )
    {
      leaf_list [ leaf_count ++ ] = l ;
      leaf_list [ leaf_count ] = NULL ;
    }
  }

  return leaf_list ;
}

/*
* NAME
*   ssgArrayTool
*
* DESCRIPTION
*   Process the graph and convert all leaf entities into vertex arrays.
*
*   Each vertex is described by a single index (instead of different
*   indices into the v, vc, vt-arrays). This may introduce new redundant
*   vertex data into the data set, which may seem silly. However, when
*   rendering through OpenGL, it is in most cases the optimal solution
*   to use indexed vertex arrays, which only have ONE index for all
*   the vertex data.
*
* INPUTS
*
* ent - the entity to process
*
* vtol - an array of 3 floats used to specify tolerances
*   vtol[0] - tolerance value for vertices
*   vtol[1] - tolerance for colours
*   vtol[2] - tolerance for texture coordinates
*
* make_normals - if true then averaged vertex normals are computed
*/
void ssgArrayTool ( ssgEntity *ent, float* vtol, bool make_normals )
{
  current_vtol = vtol? vtol: optimise_vtol ;

  ssgLeaf** leaf_list = build_leaf_list ( ent ) ;
  for ( int i = 0 ; leaf_list [ i ] != NULL ; i ++ )
  {
    ssgLeaf  *l = leaf_list [ i ] ;
    ssgState *s = l -> getState() ;
    int       cf = l -> getCullFace() ;
    
    OptVertexList list ( s, cf ) ;
    list . add ( l ) ;
    
    if ( list . tnum > 0 )  /* If all the triangles are degenerate maybe */
    {
      ssgIndexArray    *new_index     = new ssgIndexArray    ( list . tnum * 3 ) ;
      ssgVertexArray   *new_coords    = new ssgVertexArray   ( list . vnum ) ;
      ssgTexCoordArray *new_texcoords = new ssgTexCoordArray ( list . vnum ) ;
      ssgColourArray   *new_colours   = new ssgColourArray   ( list . vnum ) ;
      ssgNormalArray   *new_normals   = 0 ;

      if ( make_normals )
      {
        list . makeNormals () ;
        new_normals = new ssgNormalArray ( list . vnum ) ;
      }
      
      for ( int t = 0 ; t < list . tnum ; t++ )
      {
        new_index -> add ( list . tlist [ t*3+ 0 ] ) ;
        new_index -> add ( list . tlist [ t*3+ 1 ] ) ;
        new_index -> add ( list . tlist [ t*3+ 2 ] ) ;
      }

      for ( int v = 0 ; v < list . vnum ; v++ )
      {
        new_coords   -> add ( list . vlist[ v ]->vertex   ) ;
        new_texcoords-> add ( list . vlist[ v ]->texcoord ) ;
        new_colours  -> add ( list . vlist[ v ]->colour   ) ;
        if ( make_normals )
          new_normals  -> add ( list . vlist[ v ]->normal ) ;
      }
      
      ssgVtxArray *new_varray = new ssgVtxArray ( GL_TRIANGLES,
        new_coords, new_normals, new_texcoords, new_colours, new_index ) ;
      new_varray -> setState ( list.state ) ;
      new_varray -> setCullFace ( list.cullface ) ;
      
      ssgBranch *p ;

      /*
      Add the new leaf
      */
      for ( p = l -> getParent ( 0 ) ; p != NULL ;
        p = l -> getNextParent () )
        p -> addKid ( new_varray ) ;

      /*
      Remove the old leaf
      */
      for ( p = new_varray -> getParent ( 0 ) ; p != NULL ;
        p = new_varray -> getNextParent () )
        p -> removeKid ( l ) ;
    }
  }
  delete[] leaf_list ;
  
  ent -> recalcBSphere () ;
}

void ssgStripify ( ssgEntity *ent )
{
  current_vtol = optimise_vtol ;

  /*
    Walk down until we find a leaf node, then
    back up one level, collect all the ssgVtxTables
    into one big heap and triangulate them.
  */

  if ( ent -> isAKindOf ( ssgTypeLeaf () ) )
    return ;

  ssgBranch *b_ent = (ssgBranch *) ent ;

  /*
    Count number of unique materials (and cull-facedness)
    - make a list of them.  Recursively stripify non-leaf nodes.
  */

  int stot = 0 ;
  ssgState **slist = new ssgState *[ b_ent -> getNumKids () ] ;
  int      *cflist = new int       [ b_ent -> getNumKids () ] ;

  for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
				 k = b_ent -> getNextKid () )
  {
    if ( k -> isAKindOf ( ssgTypeVtxTable () ) )
    {
      int i ;
      ssgState *s = ((ssgLeaf *) k ) -> getState() ;
      int       c = ((ssgLeaf *) k ) -> getCullFace() ;

      for ( i = 0 ; i < stot ; i++ )
	if ( s == slist [ i ] && c == cflist [ i ] )
	  break ;

      if ( i >= stot )
      {
	slist  [ i ] = s ;
	cflist [ i ] = c ;
	stot++ ;
      }
    }
    else
    if ( k -> isAKindOf ( ssgTypeBranch () ) )
      ssgStripify ( k ) ;
  }

  /*
    Now, for each unique state, grab all the VtxTable leaf nodes
    and smoosh them into one.
  */

  for ( int i = 0 ; i < stot ; i++ )
  {
    /*
      Put it into a triangle-oriented structure and
      then do stripifying and average normal generation.

      Ick!
    */

    OptVertexList list ( slist [ i ], cflist [ i ] ) ;

    ssgEntity *k = b_ent -> getKid ( 0 ) ;
    
    while ( k != NULL )
    {
      if ( k -> isAKindOf ( ssgTypeVtxTable () ) &&
           ((ssgLeaf *) k ) -> getState() == slist [ i ] &&
           ((ssgLeaf *) k ) -> getCullFace() == cflist [ i ] )
      {
        list . add ( (ssgVtxTable *) k ) ;
        b_ent -> removeKid ( k ) ;
        k = b_ent -> getKid ( 0 ) ;
      }
      else
        k = b_ent -> getNextKid () ;
    }

    if ( list . tnum == 0 )  /* If all the triangles are degenerate maybe */
      continue ;

    /*
      So, now we have all the important information sucked out of
      all those nodes and safely tucked away in the OptVertexList 

      Let's take this opportunity to compute vertex normals.
    */

    list . makeNormals () ;

    /*
      Find the least connected triangle.
      Use it as the starting point.
    */

    short tleast, nleast ;

    while ( list . getLeastConnected ( & tleast, & nleast ) )
    {
      /* OK, we have our starting point - follow where it
         leads - but which way to start? We need two vertices
         with at least two references - and not the least
         referenced vertex please. */

      short *new_vlist = new short [ list.tnum * 3 ] ;
      short new_vc = 0 ;

      int striplength = 0 ;

      if ( nleast == list.tlist[tleast*3+0] )
      {
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 0 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 1 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 2 ] ;
        list . follow ( tleast, 1, 2, FALSE, &striplength, new_vlist, & new_vc ) ;
      }
      else
      if ( nleast == list.tlist[tleast*3+1] )
      {
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 1 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 2 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 0 ] ;
        list . follow ( tleast, 2, 0, FALSE, &striplength, new_vlist, & new_vc ) ;
      }
      else
      if ( nleast == list.tlist[tleast*3+2] )
      {
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 2 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 0 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 1 ] ;
        list . follow ( tleast, 0, 1, FALSE, &striplength, new_vlist, & new_vc ) ;
      }
      else
        ulSetError ( UL_WARNING, "Tleast doesn't contain nleast!" ) ;

      ssgVertexArray   *new_coords    = new ssgVertexArray   ( new_vc ) ;
      ssgNormalArray   *new_normals   = new ssgNormalArray   ( new_vc ) ;
      ssgTexCoordArray *new_texcoords = new ssgTexCoordArray ( new_vc ) ;
      ssgColourArray   *new_colours   = new ssgColourArray   ( new_vc ) ;

      for ( int m = 0 ; m < new_vc ; m++ )
      {
        new_coords   -> add ( list.vlist[new_vlist[m]]->vertex   ) ;
        new_normals  -> add ( list.vlist[new_vlist[m]]->normal   ) ;
        new_texcoords-> add ( list.vlist[new_vlist[m]]->texcoord ) ;
        new_colours  -> add ( list.vlist[new_vlist[m]]->colour   ) ;
      }

      delete new_vlist ;

      ssgVtxTable *new_vtable = new ssgVtxTable ( GL_TRIANGLE_STRIP,
                    new_coords, new_normals, new_texcoords, new_colours ) ;
      new_vtable -> setState ( list.state ) ;
      new_vtable -> setCullFace ( list.cullface ) ;

      b_ent -> addKid ( new_vtable ) ;
    }
  }

  delete  slist ;
  delete cflist ;
}


/*
  These routines are essentially non-realtime tree optimisations.
*/

static void strip ( ssgEntity *ent )
{
  /*
    Strip off all branches with no kids - and snip out all
    simple branches with just one kid.
  */

  if ( ! ent -> isAKindOf ( ssgTypeBranch () ) )
    return ;

  ssgBranch *b_ent = (ssgBranch *) ent ;

  for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
				 k = b_ent -> getNextKid () )
    strip ( k ) ;

  if ( ! ent -> isA ( ssgTypeBranch () ) )
    return ;

  if ( b_ent -> getNumKids () == 1 )
  {
    for ( ssgBranch *p = b_ent -> getParent ( 0 ) ; p != NULL ;
				  p = b_ent -> getNextParent () )
       p -> addKid ( b_ent -> getKid ( 0 ) ) ; 

    b_ent -> removeKid ( 0 ) ;
  }

  if ( b_ent -> getNumKids () == 0 )
  {
    for ( ssgBranch *p = b_ent -> getParent ( 0 ) ;
				p != NULL ; p = b_ent -> getNextParent () )
       p -> removeKid ( b_ent ) ; 
  }
}


static void flatten ( ssgEntity *ent, sgMat4 m )
{
  /*
    Move all transforms down to the leaf nodes and
    then multiply them out. You need to strip() the
    tree after calling this.
  */

  if ( ent -> isAKindOf ( ssgTypeLeaf () ) )
  {
    ((ssgLeaf *) ent) -> transform ( m ) ;
    return ;
  }

  if ( ent -> isAKindOf ( ssgTypeCutout () ) )
  {
/*
    ulSetError ( UL_WARNING, "ssgFlatten: Can't flatten subtrees containing Cutout nodes" ) ; 
*/
    return ;
  }

  /*
    Transforms with userdata are assumed to be
    special and cannot be flattened.
  */

  if ( ent -> isAKindOf ( ssgTypeTransform () ) &&
       ent -> getUserData () == NULL )
  {
    ssgTransform *t_ent = (ssgTransform *) ent ;
    ssgBranch *br = new ssgBranch () ;
    sgMat4 tmp1, tmp2 ;

    t_ent -> getTransform ( tmp1 ) ;
    sgCopyMat4 ( tmp2, m ) ;
    sgPreMultMat4 ( tmp2, tmp1 ) ;

    ssgEntity *k ;

    while ( (k = t_ent -> getKid ( 0 )) != NULL )
    {
      flatten ( k, tmp2 ) ;
      br -> addKid ( k ) ;
      t_ent -> removeKid ( k ) ;
    }

    br    -> recalcBSphere () ;
    t_ent -> recalcBSphere () ;

    for ( ssgBranch *p = t_ent -> getParent ( 0 ) ;
		     p != NULL ; p = t_ent -> getNextParent () )
    {
      p -> addKid ( br ) ;
      p -> recalcBSphere () ;
    }
  }
  else
  {
    ssgBranch *b_ent = (ssgBranch *) ent ;

    for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
                                      k = b_ent -> getNextKid () )
      flatten ( k, m ) ;
  }
}


void ssgFlatten ( ssgEntity *ent )
{
  sgMat4 m ;
  sgMakeIdentMat4 ( m ) ;

  flatten ( ent, m ) ;
  strip   ( ent ) ;

  ent -> recalcBSphere () ;
}


/*
* NAME
*   ssgTransTool
*
* DESCRIPTION
*   Apply a transform (translate, rotate, scale) to all verticies of a graph.
*
* INPUTS
*   ent   -- the entity to process
*   trans -- transform
*/
void ssgTransTool ( ssgEntity *ent, const sgMat4 trans )
{
  sgMat4 m ;
  sgCopyMat4 ( m, trans ) ;

  flatten ( ent, m ) ;
  strip   ( ent ) ;

  ent -> recalcBSphere () ;
}
