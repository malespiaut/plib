
#include "ssgLocal.h"

#define HL_DELTA 0.04f

void ssgVtxArray::copy_from ( ssgVtxArray *src, int clone_flags )
{
  ssgVtxTable::copy_from ( src, clone_flags ) ;

  ssgDeRefDelete ( indices ) ;

  if ( src->indices != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    indices = (ssgIndexArray *)( src -> indices -> clone ( clone_flags )) ;
  else
    indices = src -> indices ;

  if ( indices != NULL )
    indices -> ref () ;
}

ssgBase *ssgVtxArray::clone ( int clone_flags )
{
  ssgVtxArray *b = new ssgVtxArray ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgVtxArray::ssgVtxArray () : ssgVtxTable()
{
  type |= SSG_TYPE_VTXARRAY ;
  indices = NULL ;
}


ssgVtxArray::ssgVtxArray ( GLenum ty,
                ssgVertexArray   *vl,
                ssgNormalArray   *nl,
                ssgTexCoordArray *tl,
                ssgColourArray   *cl,
                ssgIndexArray    *il ) : ssgVtxTable( ty, vl, nl, tl, cl )
{
  type |= SSG_TYPE_VTXARRAY ;

  indices = (il!=NULL) ? il : new ssgIndexArray () ;

  indices -> ref () ;
}


ssgVtxArray::~ssgVtxArray ()
{
  ssgDeRefDelete ( indices      ) ;
} 


void ssgVtxArray::drawHighlight ( sgVec4 colour )
{
  _ssgForceLineState () ;

  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;

  glDisableClientState ( GL_COLOR_ARRAY         ) ;
  glDisableClientState ( GL_NORMAL_ARRAY        ) ;
  glDisableClientState ( GL_TEXTURE_COORD_ARRAY ) ;
  glEnableClientState  ( GL_VERTEX_ARRAY        ) ;

  glVertexPointer ( 3, GL_FLOAT, 0, vertices->get(0) ) ;

  glPushAttrib  ( GL_POLYGON_BIT ) ;
  glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
  glColor4fv ( colour ) ;
  int i = getNumIndices ();
  short *ii = indices->get(0);
  glDrawElements ( gltype, i, GL_UNSIGNED_SHORT, ii ) ;
  glPopAttrib () ;
  glPopClientAttrib () ;
  glEnable ( GL_DEPTH_TEST ) ;
}


void ssgVtxArray::drawHighlight ( sgVec4 colour, int i )
{
  _ssgForceLineState () ;

  if ( i < 0 || i >= getNumIndices () )
    return ;

  int ii = *( indices->get(i) );
  sgVec3 *vx = (sgVec3 *) vertices -> get(ii) ;

  float x = vx[0][0] ;
  float y = vx[0][1] ;
  float z = vx[0][2] ;

  sgVec3 t[6] ;
  sgSetVec3 ( t[0], x-HL_DELTA,y,z ) ;
  sgSetVec3 ( t[1], x+HL_DELTA,y,z ) ;
  sgSetVec3 ( t[2], x,y-HL_DELTA,z ) ;
  sgSetVec3 ( t[3], x,y+HL_DELTA,z ) ;
  sgSetVec3 ( t[4], x,y,z-HL_DELTA ) ;
  sgSetVec3 ( t[5], x,y,z+HL_DELTA ) ;

  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;
  glDisableClientState ( GL_COLOR_ARRAY         ) ;
  glDisableClientState ( GL_NORMAL_ARRAY        ) ;
  glDisableClientState ( GL_TEXTURE_COORD_ARRAY ) ;
  glEnableClientState  ( GL_VERTEX_ARRAY        ) ;

  glVertexPointer ( 3, GL_FLOAT, 0, t ) ;
  glColor4fv ( colour ) ;
  glLineWidth ( 4.0f ) ;
  glDrawArrays ( GL_LINES, 0, 6 ) ;
  glLineWidth ( 1.0f ) ;
  glPopClientAttrib ( ) ;
  glEnable ( GL_DEPTH_TEST ) ;
}



#ifdef _SSG_USE_PICK
void ssgVtxArray::pick ( int baseName )
{
  int i ;
  int num_vertices  = getNumIndices () ;

  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;
  glEnableClientState ( GL_VERTEX_ARRAY ) ;
  glVertexPointer ( 3, GL_FLOAT, 0, vertices->get(0) ) ;

  /* Test the entire primitive. */

  glPushName ( baseName ) ;
  short *ii = indices->get(0);
  glDrawElements ( gltype, num_vertices, GL_UNSIGNED_SHORT, ii ) ;

  /* Then test each vertex in turn */

  for ( i = 0 ; i < num_vertices ; i++ )
  {
    int ii = *( indices->get(i) );
    glLoadName ( baseName + i + 1 ) ;
    glBegin ( GL_POINTS ) ;
    glArrayElement ( ii ) ;
    glEnd () ;
  }

  glPopName () ;

  glPopClientAttrib ( ) ;
}
#endif // #ifdef _SSG_USE_PICK


void ssgVtxArray::draw_geometry ()
{
  int num_colours   = getNumColours   () ;
  int num_normals   = getNumNormals   () ;
  int num_texcoords = getNumTexCoords () ;

  sgVec3 *nm = (sgVec3 *) normals   -> get(0) ;
  sgVec4 *cl = (sgVec4 *) colours   -> get(0) ;

  if ( num_colours == 0 ) glColor4f   ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
  if ( num_colours == 1 ) glColor4fv  ( cl [ 0 ] ) ;
  if ( num_normals == 1 ) glNormal3fv ( nm [ 0 ] ) ;
  
  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;

  if ( num_colours > 1 )
  {
    glEnableClientState ( GL_COLOR_ARRAY ) ;
    glColorPointer ( 4, GL_FLOAT, 0, colours->get(0) ) ;
  }
  if ( num_normals > 1 )
  {
    glEnableClientState ( GL_NORMAL_ARRAY ) ;
    glNormalPointer ( GL_FLOAT, 0, normals->get(0) ) ;
  }
  if ( num_texcoords > 1 )
  {
    glEnableClientState ( GL_TEXTURE_COORD_ARRAY ) ;
    glTexCoordPointer ( 2, GL_FLOAT, 0, texcoords->get(0) ) ;
  }
  glEnableClientState ( GL_VERTEX_ARRAY ) ;
  glVertexPointer ( 3, GL_FLOAT, 0, vertices->get(0) ) ;

  int i = getNumIndices ();
  short *ii = indices->get(0);
  glDrawElements ( gltype, i, GL_UNSIGNED_SHORT, ii ) ;

  glPopClientAttrib ( ) ;
}

 
void ssgVtxArray::getTriangle ( int n, short *v1, short *v2, short *v3 )
{
  short vv1, vv2, vv3 ;

  ssgVtxTable::getTriangle ( n, &vv1, &vv2, &vv3 ) ;

  *v1 = *( indices -> get ( vv1 ) ) ;
  *v2 = *( indices -> get ( vv2 ) ) ;
  *v3 = *( indices -> get ( vv3 ) ) ;
}


int ssgVtxArray::getNumTriangles ()
{
  switch ( getPrimitiveType () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
      return getNumIndices() - 2 ;

    case GL_TRIANGLES :
      return getNumIndices() / 3 ;

    case GL_TRIANGLE_STRIP :
      return getNumIndices() - 2 ;

    case GL_QUADS :
      return ( getNumIndices() / 4 ) * 2 ;

    case GL_QUAD_STRIP :
      return ( ( getNumIndices() - 2 ) / 2 ) * 2 ;

    default : break ;
  }

  return 0 ;   
}

int  ssgVtxArray::getNumLines ()
{
  switch ( getPrimitiveType () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
    case GL_TRIANGLES :
    case GL_TRIANGLE_STRIP :
    case GL_QUADS :
    case GL_QUAD_STRIP :
    case GL_POINTS :
      return 0;
		case GL_LINES :   
			// wk: FIXME: check the 3 following formulas. I don't have an OpenGL bokk at hand currently :-(
			return getNumIndices ()/2;
    case GL_LINE_LOOP :
    	return getNumIndices ();
    case GL_LINE_STRIP :
    	return getNumIndices ()-1;
    default : break ;
  }
  assert(false); /* Should never get here  */
  return 0 ;   
}

void ssgVtxArray::getLine ( int n, short *v1, short *v2 )
{ 
	assert( n>=0 );
  switch ( getPrimitiveType () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
    case GL_TRIANGLES :
    case GL_TRIANGLE_STRIP :
    case GL_QUADS :
    case GL_QUAD_STRIP :
    case GL_POINTS :
      assert(false);
		case GL_LINES :   
			// wk: FIXME: check the 3 following formulas. I don't have an OpenGL bokk at hand currently :-(
			assert ( 2*n+1 < getNumIndices() );
			*v1 = *getIndex( 2*n );
			*v2 = *getIndex( 2*n+1 );
			return ;
    case GL_LINE_LOOP :
			assert ( n < getNumIndices() );
			*v1 = *getIndex( n );
			if ( n == getNumIndices()-1 )
			  *v2 = *getIndex( 0 );
			else
				*v2 = *getIndex( n+1 );
			return ;
    case GL_LINE_STRIP :
    	assert ( n < getNumIndices()-1 );
			*v1 = *getIndex( n );
			*v2 = *getIndex( n+1 );
			return;
    default :
			break ;
  }
  assert(false); /* Should never get here  */
  return ;   
}


void ssgVtxArray::print ( FILE *fd, char *indent, int how_much )
{
	char in [ 100 ] ;

	if ( how_much == 0 ) 
		return; // dont print anything
  
  sprintf ( in, "%s  ", indent );
	
	// wk: Why were these 2 lines commented out?:
  ssgVtxTable::print ( fd, indent, how_much ) ;
  indices   -> print ( fd, in, how_much ) ;
}


int ssgVtxArray::load ( FILE *fd )
{
  if ( ! ssgVtxTable::load(fd) ||
       ! _ssgLoadObject ( fd, (ssgBase **) &indices, ssgTypeIndexArray () ) )
    return FALSE ;

  if ( indices != NULL)
    indices -> ref () ;

  return TRUE ;
}


int ssgVtxArray::save ( FILE *fd )
{
  if ( ! ssgVtxTable::save(fd) ||
       ! _ssgSaveObject ( fd, indices ) )
    return FALSE ;
   
  return TRUE ;
}
