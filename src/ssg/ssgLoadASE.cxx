//
// ASE ( 3DSMAX ASCII EXPORT Version 2.00 ) loader for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in Feb-2000
//

#include "ssgLocal.h"
#include "ssgParser.h"

#define u32 unsigned int
#define f32 float
#define cchar const char


struct aseVertexBuffer
{
  bool use_flag ;
  int index ;
  sgVec3 v ;
  sgVec2 tv ;
  sgVec3 cv ;
} ;


struct aseFace
{
   u32 v[3];
   u32 tv[3];
   u32 cv[3];
   u32 sub_index;
};


struct aseMesh
{
  u32 num_faces ;
  u32 num_verts ;
  u32 num_tverts ;
  u32 num_cverts ;
  
  aseFace* faces ;
  sgVec3* verts ;
  sgVec2* tverts ;
  sgVec3* cverts ;

  aseMesh();
  ~aseMesh();
};


struct aseMaterial
{
  char* name ;
  u32 mat_index ;
  u32 sub_index ;
  bool sub_flag ;
   
  sgVec4 amb ;
  sgVec4 diff ;
  sgVec4 spec ;
  f32 shine ;
  f32 transparency ;
  
  char* tfname ;
  sgVec2 texrep ;
  sgVec2 texoff ;
};


enum { MAX_MATERIALS = 1000 };
static aseMaterial** materials ;
static u32 num_materials ;

enum { MAX_FRAMES = 256 };
aseMesh* mesh_list[ MAX_FRAMES ];
int mesh_count = 0 ;

static u32 first_frame ;
static u32 last_frame ;
static u32 frame_speed ;
static u32 ticks_per_frame ;
static u32 num_frames ;

static _ssgParserSpec parser_spec =
{
   "\r\n\t ",  //delim_chars
   "{",        //open_brace_chars
   "}",        //close_brace_chars
   '"',        //quote_char
   0           //comment_char
} ;

static _ssgParser parser;
static ssgBranch* top_branch;

aseMesh::aseMesh()
{
  memset(this,0,sizeof(aseMesh));
}


aseMesh::~aseMesh()
{
  delete[] faces;
  delete[] verts;
  delete[] tverts;
  delete[] cverts;
}


static u32 count_sub_materials( u32 mat_index )
{
  u32 count = 0 ;
  for ( u32 i=0; i<num_materials; i++ )
  {
    aseMaterial* mat = materials[ i ] ;
    if ( mat->mat_index == mat_index && mat->sub_flag )
      count ++;
  }
  return count ;
}


static aseMaterial* find_material( u32 mat_index, u32 sub_index )
{
  u32 i;
  
  //find sub-material
  for ( i=0; i<num_materials; i++ )
  {
    aseMaterial* mat = materials[ i ] ;
    if ( mat->mat_index == mat_index && mat->sub_index == sub_index )
      return(mat);
  }
  
  //just match material #
  for ( i=0; i<num_materials; i++ )
  {
    aseMaterial* mat = materials[ i ] ;
    if ( mat->mat_index == mat_index )
      return(mat);
  }
  
  parser.error("unknown material #%d",mat_index);
  return(0);
}


static ssgSimpleState* get_state( aseMaterial* mat )
{
  ssgSimpleState *st = new ssgSimpleState () ;

//  st -> setMaterial ( GL_AMBIENT, mat -> amb ) ;
//  st -> setMaterial ( GL_DIFFUSE, mat -> diff ) ;
  st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
  st -> setShininess ( mat -> shine ) ;

  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  st -> enable  ( GL_LIGHTING       ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  if ( mat -> transparency > 0.0f )
  {
    st -> disable ( GL_ALPHA_TEST ) ;
    st -> enable  ( GL_BLEND ) ;
    st -> setTranslucent () ;
  }
  else
  {
    st -> disable ( GL_BLEND ) ;
    st -> setOpaque () ;
  }

  return st ;
}


static void parse_map( aseMaterial* mat )
{
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*BITMAP"))
    {
      if ( mat->tfname != NULL )
        parser.error("multiple textures for material: %s",mat->name);
      else
      {
         char* fname = parser.parseString("bitmap filename") ;

         //strip existing directory from fname
         char* slash = strrchr ( fname, '/' ) ;
         if ( !slash )
           slash = strrchr ( fname, '\\' ) ; //for dos
         if ( slash )
           fname = slash + 1 ;

         mat->tfname = new char [ strlen(fname)+1 ] ;
         strcpy ( mat->tfname, fname ) ;
      }
    }
    else if (!strcmp(token,"*UVW_U_TILING"))
    {
      mat->texrep[0] = parser.parseFloat("tiling.u");
    }
    else if (!strcmp(token,"*UVW_V_TILING"))
    {
      mat->texrep[1] = parser.parseFloat("tiling.v");
    }
    else if (!strcmp(token,"*UVW_U_OFFSET"))
    {
      mat->texoff[0] = parser.parseFloat("offset.u");
    }
    else if (!strcmp(token,"*UVW_V_OFFSET"))
    {
      mat->texoff[1] = parser.parseFloat("offset.v");
    }
  }
}


static void parse_material( u32 mat_index, u32 sub_index, cchar* mat_name )
{
  if ( num_materials >= MAX_MATERIALS )
  {
    parser.error( "too many materials" );

    // skip material definition
    int startLevel = parser.level;
    while (parser.getLine( startLevel ) != NULL)
      ;
    return ;
  }
  aseMaterial* mat = new aseMaterial;
  materials [ num_materials++ ] = mat ;

  memset ( mat, 0, sizeof(aseMaterial) ) ;
  mat->mat_index = mat_index ;
  mat->sub_index = sub_index ;
  mat->sub_flag = ( mat_name != 0 ) ;
  mat->texrep[0] = 1.0f ;
  mat->texrep[1] = 1.0f ;
  mat->texoff[0] = 0.0f ;
  mat->texoff[1] = 0.0f ;
  
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*MATERIAL_NAME"))
    {
      char* name = parser.parseString("mat name");
      if ( mat->sub_flag )
      {
        char buff [ 256 ] ;
        sprintf( buff, "%s, sub#%d", mat_name, sub_index );

        mat->name = new char [ strlen(buff)+1 ] ;
        strcpy ( mat->name, buff ) ;
      }
      else
      {
        mat->name = new char [ strlen(name)+1 ] ;
        strcpy ( mat->name, name ) ;
      }
    }
    else if (!strcmp(token,"*MATERIAL_AMBIENT"))
    {
      mat->amb[ 0 ] = parser.parseFloat("amb.r");
      mat->amb[ 1 ] = parser.parseFloat("amb.g");
      mat->amb[ 2 ] = parser.parseFloat("amb.b");
      mat->amb[ 3 ] = 1.0f;
    }
    else if (!strcmp(token,"*MATERIAL_DIFFUSE"))
    {
      mat->diff[ 0 ] = parser.parseFloat("diff.r");
      mat->diff[ 1 ] = parser.parseFloat("diff.g");
      mat->diff[ 2 ] = parser.parseFloat("diff.b");
      mat->diff[ 3 ] = 1.0f;
    }
    else if (!strcmp(token,"*MATERIAL_SPECULAR"))
    {
      mat->spec[ 0 ] = parser.parseFloat("spec.r");
      mat->spec[ 1 ] = parser.parseFloat("spec.g");
      mat->spec[ 2 ] = parser.parseFloat("spec.b");
      mat->spec[ 3 ] = 1.0f;
    }
    else if (!strcmp(token,"*MATERIAL_SHINE"))
    {
      mat->shine = parser.parseFloat("shine");
    }
    else if (!strcmp(token,"*MATERIAL_TRANSPARENCY"))
    {
      mat->transparency = parser.parseFloat("transparency");
    }
    else if (!strcmp(token,"*MAP_DIFFUSE"))
    {
      //Need: what about MAP_GENERIC, MAP_AMBIENT, etc??
      parse_map( mat );
    }
    else if (!strcmp(token,"*SUBMATERIAL"))
    {
      u32 sub_index = parser.parseInt("sub mat #");
      parse_material( mat_index, sub_index, mat->name );
    }
  }

  //parser.message("material: %s (%s)",mat->name,mat->tfname);
}


static void parse_material_list()
{
  if ( num_materials )
    parser.error("multiple material lists");

  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*MATERIAL"))
    {
      u32 mat_index = parser.parseInt("mat #");
      parse_material( mat_index, 9999, NULL );
    }
  }
}


static void parse_mesh()
{
  aseMesh* mesh = NULL ;

  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if ( mesh == NULL )
    {
      u32 frame = MAX_FRAMES ;

      if (!strcmp(token,"*TIMEVALUE"))
      {
        u32 time = parser.parseInt("time");
        frame = (time + (ticks_per_frame-1)) / ticks_per_frame - first_frame;
      }
      else
      {
        parser.error("missing *TIMEVALUE");
        frame = MAX_FRAMES ;
      }

      if ( frame >= MAX_FRAMES || mesh_list [ frame ] != NULL )
      {
        //ignore this mesh
        while (parser.getLine( startLevel )) ;
        return;
      }

      mesh = new aseMesh ;
      mesh_list [ frame ] = mesh ;
      mesh_count ++ ;
    }
    else if (!strcmp(token,"*MESH_NUMFACES"))
    {
      if (mesh -> faces)
        parser.error("%s already seen",token);
      else
      {
        mesh -> num_faces = parser.parseInt("num_faces");
        mesh -> faces = new aseFace[ mesh -> num_faces ];
      }
    }
    else if (!strcmp(token,"*MESH_NUMTVFACES"))
    {
      u32 ntfaces = parser.parseInt("ntfaces");
      if (ntfaces != mesh -> num_faces)
        parser.error("NUMTFACES(%d) != NUMFACES(%d)",ntfaces,mesh -> num_faces);
    }
    else if (!strcmp(token,"*MESH_NUMCVFACES"))
    {
      u32 ncfaces = parser.parseInt("ncfaces");
      if (ncfaces != mesh -> num_faces)
        parser.error("NUMCFACES(%d) != NUMFACES(%d)",ncfaces,mesh -> num_faces);
    }
    else if (!strcmp(token,"*MESH_NUMVERTEX"))
    {
      if (mesh -> verts)
        parser.error("%s already seen",token);
      else
      {
        mesh -> num_verts = parser.parseInt("num_verts");
        mesh -> verts = new sgVec3[ mesh -> num_verts ];
      }
    }
    else if (!strcmp(token,"*MESH_NUMTVERTEX"))
    {
      if (mesh -> tverts)
        parser.error("%s already seen",token);
      else
      {
        mesh -> num_tverts = parser.parseInt("num_tverts");
        if (mesh -> num_tverts)
          mesh -> tverts = new sgVec2[ mesh -> num_tverts ];
      }
    }
    else if (!strcmp(token,"*MESH_NUMCVERTEX"))
    {
      if (mesh -> cverts)
        parser.error("%s already seen",token);
      else
      {
        mesh -> num_cverts = parser.parseInt("num_cverts");
        if (mesh -> num_cverts)
          mesh -> cverts = new sgVec3[ mesh -> num_cverts ];
      }
    }
    else if (!strcmp(token,"*MESH_FACE"))
    {
      u32 index = parser.parseInt("face #");
      if (index >= mesh -> num_faces)
        parser.error("bad face #");
      else
      {
        aseFace& face = mesh -> faces[ index ];

        parser.expect("A:");
        face.v[0] = parser.parseInt("face.v[0]");
        parser.expect("B:");
        face.v[1] = parser.parseInt("face.v[1]");
        parser.expect("C:");
        face.v[2] = parser.parseInt("face.v[2]");

        //search for other flags
        while ( (token = parser.parseToken(0)) != 0 )
        {
          if ( strcmp(token,"*MESH_MTLID") == 0 )
          {
            face.sub_index = parser.parseInt("mat #");
          }
        }
      }
    }
    else if (!strcmp(token,"*MESH_TFACE"))
    {
      u32 index = parser.parseInt("tface #");
      if (index >= mesh -> num_faces)
        parser.error("bad tface #");
      else
      {
        aseFace& face = mesh -> faces[ index ];

        face.tv[0] = parser.parseInt("tface.tv[0]");
        face.tv[1] = parser.parseInt("tface.tv[1]");
        face.tv[2] = parser.parseInt("tface.tv[2]");
      }
    }
    else if (!strcmp(token,"*MESH_CFACE"))
    {
      u32 index = parser.parseInt("cface #");
      if (index >= mesh -> num_faces)
        parser.error("bad cface #");
      else
      {
        aseFace& face = mesh -> faces[ index ];

        face.cv[0] = parser.parseInt("tface.cv[0]");
        face.cv[1] = parser.parseInt("tface.cv[1]");
        face.cv[2] = parser.parseInt("tface.cv[2]");
      }
    }
    else if (!strcmp(token,"*MESH_VERTEX"))
    {
      u32 index = parser.parseInt("vertex #");
      if (index >= mesh -> num_verts)
        parser.error("bad vertex #");
      else
      {
        sgVec3& vert = mesh -> verts[ index ];
      
        vert[0] = parser.parseFloat("vert.x");
        vert[1] = parser.parseFloat("vert.y");
        vert[2] = parser.parseFloat("vert.z");
      }
    }
    else if (!strcmp(token,"*MESH_TVERT"))
    {
      u32 index = parser.parseInt("tvertex #");
      if (index >= mesh -> num_tverts)
        parser.error("bad tvertex #");
      else
      {
        sgVec2& tvert = mesh -> tverts[ index ];
      
        tvert[0] = parser.parseFloat("tvert.x");
        tvert[1] = parser.parseFloat("tvert.y");
      }
    }
    else if (!strcmp(token,"*MESH_VERTCOL"))
    {
      u32 index = parser.parseInt("cvertex #");
      if (index >= mesh -> num_cverts)
        parser.error("bad cvertex #");
      else
      {
        sgVec3& cvert = mesh -> cverts[ index ];
      
        cvert[0] = parser.parseFloat("cvert.x");
        cvert[1] = parser.parseFloat("cvert.y");
        cvert[2] = parser.parseFloat("cvert.z");
      }
    }
  }
}


static ssgLeaf* add_mesh( cchar* mesh_name, aseMesh* mesh, u32 mat_index, u32 sub_index )
{
  aseMaterial* mat = find_material ( mat_index, sub_index ) ;
  if ( mat == NULL )
     return NULL ;

  //compute number of faces
  u32 num_faces = mesh -> num_faces ;
  if ( mat->sub_flag )
  {
    num_faces = 0 ;
    aseFace* face = mesh -> faces ;
    for ( u32 i=0; i<mesh -> num_faces; i++, face++ )
    {
      if ( face->sub_index == mat->sub_index )
        num_faces ++ ;
    }
  }
  if ( num_faces == 0 )
    return NULL;

  u32 i ;

  //allocate map_index array
  int* map_index = new int [ mesh -> num_faces * 3 ] ;

  //allocate the vertex list
  u32 max_verts = mesh -> num_verts + mesh -> num_faces * 3 ;
  aseVertexBuffer* vert_list = new aseVertexBuffer [ max_verts ] ;
  
  //mark each vertex as *not* used
  aseVertexBuffer* vert = vert_list ;
  for ( i=0; i < max_verts; i++, vert++ )
  {
    vert -> use_flag = false ;
  }

  u32 extra_verts = 0 ;

  //build the vertex list
  aseFace* face = mesh -> faces ;
  for ( i=0; i<mesh -> num_faces; i++, face++ )
  {
    if ( mat->sub_flag && face->sub_index != mat->sub_index )
      continue ;
    
    for ( u32 j=0; j<3; j++ )
    {
      int k = i*3+j;

      map_index [k] = face->v[j] ;
      vert = vert_list + map_index[k];

      if ( vert -> use_flag )
      {
        //check for match
        bool match = true ;
        if ( mesh -> tverts &&
          sgCompareVec2 ( vert -> tv, mesh -> tverts[ face->tv[j] ], 0.0f ) != 0 )
          match = false ;
        else if ( mesh -> cverts &&
          sgCompareVec3 ( vert -> cv, mesh -> cverts[ face->cv[j] ], 0.0f ) != 0 )
          match = false ;
        if ( match )
          continue ;  //texcoord and color matches other vertex

        extra_verts ++ ;

        map_index [k] = mesh -> num_verts + k ;
        vert = vert_list + map_index[k];
      }

      //add the vertex
      vert -> use_flag = true;
      sgCopyVec3 ( vert -> v, mesh -> verts[ face->v[j] ] ) ;
      if ( mesh -> tverts )
        sgCopyVec2 ( vert -> tv, mesh -> tverts[ face->tv[j] ] ) ;
      if ( mesh -> cverts )
        sgCopyVec3 ( vert -> cv, mesh -> cverts[ face->cv[j] ] ) ;
    }
  }

  //assign a unique index to each vertex
  int num_verts = 0 ;
  vert = vert_list;
  for ( i=0; i < max_verts; i++, vert++ )
  {
    if ( vert -> use_flag )
      vert -> index = num_verts ++;
  }
    
  //if ( extra_verts > 0 )
  //   fprintf( stdout, "%d verts; %d added\n", num_verts-extra_verts, extra_verts );
  //else
  //   fprintf( stdout, "%d verts\n", num_verts );

  //pass the data to ssg
  ssgIndexArray* il = new ssgIndexArray ( num_faces * 3 ) ;
  ssgVertexArray* vl = new ssgVertexArray ( num_verts ) ;
  ssgTexCoordArray* tl = NULL ;
  ssgColourArray* cl = NULL ;
  ssgNormalArray* nl = NULL ;
  if ( mesh -> tverts )  
    tl = new ssgTexCoordArray ( num_verts ) ;
  if ( mesh -> cverts )
    cl = new ssgColourArray ( num_verts ) ;

  //build the index list
  face = mesh -> faces ;
  for ( i=0; i<mesh -> num_faces; i++, face++ )
  {
    if ( mat->sub_flag && face->sub_index != mat->sub_index )
      continue ;
    
    for ( u32 j=0; j<3; j++ )
    {
      int k = i*3+j;
      vert = vert_list + map_index[k];

      if ( ! vert -> use_flag )
        ulSetError ( UL_FATAL, "internal error" ) ;

      il -> add ( vert -> index ) ;
    }
  }

  //copy the vertex lists
  vert = vert_list;
  for ( i=0; i < max_verts; i++, vert++ )
  {
    if ( vert -> use_flag )
    {
      vl -> add ( vert -> v ) ;

      if ( mesh -> tverts )
      {
        sgVec2 tv ;
        sgCopyVec2 ( tv, vert -> tv ) ;

        tv[1] = 1.0f - tv[1] ;

        tv[0] *= mat->texrep[0] ;
        tv[1] *= mat->texrep[1] ;
        tv[0] += mat->texoff[0] ;
        tv[1] += mat->texoff[1] ;

        tl -> add ( tv ) ;
      }

      if ( mesh -> cverts )
      {
        sgVec4 c ;
        sgCopyVec3 ( c, vert -> cv ) ;

        c[3] = 1.0f; //alpha is always one ??

        cl -> add ( c ) ;
      }
    }
  }

  delete[] vert_list ;
  delete[] map_index ;

  ssgSimpleState* st = get_state ( mat ) ;
  if ( st -> isEnabled ( GL_LIGHTING ) )
  {
    if ( cl == NULL )
    {
      sgVec4 c ;
      sgCopyVec3 ( c, mat -> diff ) ;
      c[3] = 1.0f - mat -> transparency ;
  
      cl = new ssgColourArray ( 1 ) ;
      cl -> add ( c ) ;
    }
  
    if ( vl -> getNum () >= 3 )
    {
      sgVec3 n ;
      sgMakeNormal ( n,
        vl -> get(0),
        vl -> get(1),
        vl -> get(2) ) ;
  
      nl = new ssgNormalArray ( 1 ) ;
      nl -> add ( n ) ;
    }
  }

  ssgVtxArray* leaf = new ssgVtxArray ( GL_TRIANGLES,
    vl, nl, tl, cl, il ) ;
  leaf -> setCullFace ( TRUE ) ;
  leaf -> setState ( st ) ;
  return (*_ssgCreateFunc) ( leaf, mat -> tfname, mesh_name ) ;
}


static void parse_object()
{
  char* mesh_name = 0 ;
  char* mesh_parent = 0 ;
  u32 mat_index = 0;

  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*NODE_NAME"))
    {
      if ( !mesh_name )
      {
        char* name = parser.parseString("obj name");

        mesh_name = new char [ strlen(name)+1 ] ;
        strcpy ( mesh_name, name ) ;
      }
    }
    else if (!strcmp(token,"*NODE_PARENT"))
    {
      if ( !mesh_parent )
      {
        char* name = parser.parseString("parent name");

        mesh_parent = new char [ strlen(name)+1 ] ;
        strcpy ( mesh_parent, name ) ;
      }
    }
    else if (!strcmp(token,"*TM_POS"))
    {
      //NEED: support for *TM_ROW0, ...
      sgVec3 pos ;
      pos[0] = parser.parseFloat("pos.x");
      pos[1] = parser.parseFloat("pos.y");
      pos[2] = parser.parseFloat("pos.z");
    }
    else if (!strcmp(token,"*MESH"))
    {
      parse_mesh();
    }
    else if (!strcmp(token,"*MESH_ANIMATION"))
    {
      int startLevel = parser.level;
      while ((token = parser.getLine( startLevel )) != NULL)
      {
        if (!strcmp(token,"*MESH"))
        {
          parse_mesh();
        }
      }
    }
    else if (!strcmp(token,"*TM_ANIMATION"))
    {
      //ignore this since we can't get it to work correctly
      int startLevel = parser.level;
      while ((token = parser.getLine( startLevel )) != NULL)
        ;
    }
    else if (!strcmp(token,"*MATERIAL_REF"))
    {
      mat_index = parser.parseInt("mat #");
    }
  }

  if ( mesh_parent != NULL )
     fprintf( stdout, "add_mesh: %s, parent=%s\n", mesh_name, mesh_parent ) ;
  else
     fprintf( stdout, "add_mesh: %s\n", mesh_name ) ;

  ssgEntity* mesh_entity = NULL ;

  if ( mesh_count > 1 )
  {
    //how many frames?
    int num_frames = 0 ;
    int i ;
    for ( i=0; i<MAX_FRAMES; i++ )
    {
      aseMesh* mesh = mesh_list [ i ] ;
      if ( mesh == NULL )
        num_frames ++ ;
    }

    //allocate selector
    ssgTimedSelector* selector = new ssgTimedSelector ( num_frames ) ;

    //init
    for ( i=0; i<MAX_FRAMES; i++ )
    {
      aseMesh* mesh = mesh_list [ i ] ;
      if ( mesh == NULL )
         continue ;

      u32 num_subs = count_sub_materials ( mat_index );
      if ( num_subs > 1 )
      {
        //break apart the mesh for each sub material
        ssgBranch* branch = new ssgBranch ;
        for ( u32 sub_index=0; sub_index<num_subs; sub_index++ )
        {
          ssgLeaf* leaf = add_mesh ( mesh_name, mesh, mat_index, sub_index );
          if ( leaf )
            branch -> addKid ( leaf ) ;
        }
        selector -> addKid ( branch ) ;
      }
      else
      {
        ssgLeaf* leaf = add_mesh ( mesh_name, mesh, mat_index, 0 );
        if ( leaf )
          selector -> addKid ( leaf ) ;
      }
    }
    selector -> setLimits ( 0, selector -> getNumKids()-1 );
    mesh_entity = selector ;
  }
  else if ( mesh_list [ 0 ] != NULL )
  {
    aseMesh* mesh = mesh_list [ 0 ] ;
    ssgBranch* branch = new ssgBranch ;

    u32 num_subs = count_sub_materials ( mat_index );
    if ( num_subs > 1 )
    {
      //break apart the mesh for each sub material
      for ( u32 sub_index=0; sub_index<num_subs; sub_index++ )
      {
        ssgLeaf* leaf = add_mesh ( mesh_name, mesh, mat_index, sub_index );
        if ( leaf )
          branch -> addKid ( leaf ) ;
      }
    }
    else
    {
      ssgLeaf* leaf = add_mesh ( mesh_name, mesh, mat_index, 0 );
      if ( leaf )
        branch -> addKid ( leaf ) ;
    }

    mesh_entity = branch ;
  }

  if ( mesh_entity != NULL )
  {
    //add to graph -- find parent branch
    ssgBranch* parent_branch ;
    if ( mesh_parent )
    {
      ssgEntity* found = top_branch -> getByName ( mesh_parent ) ;
      if ( found != NULL )
      {
        assert ( found -> isAKindOf ( SSG_TYPE_BRANCH ) ) ;
        parent_branch = (ssgBranch *) found ;
      }
      else
      {
        parser.error("mesh %s: parent %s not seen",mesh_name,mesh_parent);
        parent_branch = top_branch ;
      }
    }
    else
    {
      parent_branch = top_branch ;
    }

    mesh_entity -> setName ( mesh_name ) ;
    parent_branch -> addKid ( mesh_entity ) ;
  }

  /*
   *  free up memory
   */
  delete[] mesh_name;
  delete[] mesh_parent;
  for ( int i=0; i<MAX_FRAMES; i++ )
  {
    aseMesh* mesh = mesh_list [ i ] ;
    if ( mesh != NULL )
       delete mesh ;
    mesh_list [ i ] = NULL ;
  }
  mesh_count = 0 ;
}


static bool parse()
{
  materials = new aseMaterial* [ MAX_MATERIALS ];
  if ( !materials )
  {
    parser.error("not enough memory");
    return false ;
  }

  num_materials = 0 ;
  
  first_frame = 0 ;
  last_frame = 0 ;
  frame_speed = 0 ;
  ticks_per_frame = 0 ;
  num_frames = 0 ;
  
  bool firsttime = true;
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (firsttime)
    {
      if (strcmp(token,"*3DSMAX_ASCIIEXPORT"))
      {
        parser.error("not ASE format");
        return false ;
      }
      if (parser.parseInt("version number") != 200)
      {
        parser.error("invalid %s version",token);
        return false ;
      }
      firsttime = false;
    }
    else if (!strcmp(token,"*SCENE"))
    {
      int startLevel = parser.level;
      while ((token = parser.getLine( startLevel )) != NULL)
      {
        if (!strcmp(token,"*SCENE_FIRSTFRAME"))
        {
          first_frame = parser.parseInt("FIRSTFRAME #");
        }
        else if (!strcmp(token,"*SCENE_LASTFRAME"))
        {
          last_frame = parser.parseInt("LASTFRAME #");
          num_frames = last_frame - first_frame + 1;
        }
        else if (!strcmp(token,"*SCENE_FRAMESPEED"))
        {
          frame_speed = parser.parseInt("FRAMESPEED #");
        }
        else if (!strcmp(token,"*SCENE_TICKSPERFRAME"))
        {
          ticks_per_frame = parser.parseInt("TICKSPERFRAME #");
        }
      }
    }
    else if (!strcmp(token,"*MATERIAL_LIST"))
    {
      parse_material_list();
    }
    else if (!strcmp(token,"*GEOMOBJECT"))
    {
      parse_object();
    }
  }
  return true ;
}


static void parse_free()
{
  u32 i ;
  for ( i = 0 ; i < num_materials ; i++ )
  {
    delete materials[ i ] -> name ;
    delete materials[ i ] -> tfname ;
    delete materials[ i ] ;
  }
  delete[] materials ;
  materials = 0 ;
}


ssgEntity *ssgLoadASE ( const char *fname, ssgHookFunc hookfunc )
{
  (*_ssgCreateFunc) ( 0, 0, 0 ) ;  //reset

  top_branch = new ssgBranch ;
  parser.openFile( fname, &parser_spec );
  if ( !parse() )
  {
     delete top_branch ;
     top_branch = 0 ;
  }
  parse_free();
  parser.closeFile();

  (*_ssgCreateFunc) ( 0, 0, 0 ) ;  //reset
  return top_branch ;
}
