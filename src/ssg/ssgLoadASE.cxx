//
// ASE ( 3DSMAX ASCII EXPORT Version 2.00 ) loader for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in Feb-2000
//

#include "ssgLocal.h"
#include "ssgParser.h"


struct aseFace
{
   u32 v[3];
   u32 tv[3];
   u32 cv[3];
   u32 sub_index;
};


struct aseMesh
{
  char name[ 256 ] ;
  u32 time ;  //used for mesh animations
  
  u32 num_faces ;
  u32 num_verts ;
  u32 num_tverts ;
  u32 num_cverts ;
  
  aseFace* faces ;
  sgVec3* verts ;
  sgVec2* tverts ;
  sgVec3* cverts ;
  sgVec3* norms ;
  
  void init();
  void free();
};


struct aseMaterial
{
  char name[ 256 ] ;
  u32 mat_index ;
  u32 sub_index ;
  bool sub_flag ;
   
  sgVec4 amb ;
  sgVec4 diff ;
  sgVec4 spec ;
  f32 shine ;
  f32 trans ;
  
  char tfname[ 256 ] ;
  ssgTexture* tex ;
  sgVec2 texrep ;
  sgVec2 texoff ;
};


enum { MAX_MATERIALS = 1000, MAX_STATES = 1000 };
static aseMaterial** materials ;
static u32 num_materials ;
static ssgSimpleState** states ;
static u32 num_states ;

static u32 first_frame ;
static u32 last_frame ;
static u32 frame_speed ;
static u32 ticks_per_frame ;
static u32 num_frames ;

static _ssgParser parser;
static ssgBranch* current_branch;


void aseMesh::init()
{
  memset(this,0,sizeof(aseMesh));
}


void aseMesh::free()
{
  delete[] faces;
  delete[] verts;
  delete[] tverts;
  delete[] cverts;
  delete[] norms;
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


static ssgSimpleState* find_state( aseMaterial* mat )
{
  ssgTexture* tex = mat->tex ;
  
  for ( u32 i = 0 ; i < num_states ; i++ )
  {
    ssgSimpleState *st2 = states [ i ] ;

    if ( tex == NULL && st2->isEnabled ( GL_TEXTURE_2D ) )
      continue ;

    if ( tex != NULL && ! st2->isEnabled ( GL_TEXTURE_2D ) )
      continue ;

    if ( tex != NULL && tex -> getHandle() != st2 -> getTextureHandle () )
      continue ;

    if ( ! sgEqualVec4 ( mat->amb, st2->getMaterial ( GL_AMBIENT ) ) ||
         ! sgEqualVec4 ( mat->diff, st2->getMaterial ( GL_DIFFUSE ) ) ||
         ! sgEqualVec4 ( mat->spec, st2->getMaterial ( GL_SPECULAR ) ) ||
         (int)( mat->trans < 0.99 ) != st2 -> isTranslucent () ||
         mat -> shine != st2->getShininess () )
      continue ;

    return st2 ;
  }

  if ( num_states >= MAX_STATES )
  {
    parser.error( "too many states" );
    return 0 ;
  }
  ssgSimpleState *st = new ssgSimpleState () ;
  states [ num_states++ ] = st ;
  
  if ( tex != NULL )
  {
    st -> setTexture ( tex ) ;
    st -> enable     ( GL_TEXTURE_2D ) ;
  }
  else
    st -> disable    ( GL_TEXTURE_2D ) ;

  st -> setMaterial ( GL_AMBIENT, mat -> amb ) ;
  st -> setMaterial ( GL_DIFFUSE, mat -> diff ) ;
  st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
  st -> setShininess ( mat -> shine ) ;

  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  st -> enable  ( GL_LIGHTING       ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  if ( mat -> trans < 0.99f )
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
      if ( mat->tex )
        parser.error("multiple textures for material: %s",mat->name);
      else
      {
         char* fname = parser.parseString("bitmap filename") ;
         _ssgMakePath(mat->tfname,_ssgTexturePath,fname,0) ;
   
         //re-use textures when possible
         for ( u32 i = 0 ; i < (num_materials-1) ; i++ )
         {
           if ( strcmp ( mat->tfname, materials[ i ]->tfname ) == 0 )
           {
             mat->tex = materials[ i ]->tex ;
             break ;
           }
         }
   
         //load texture
         if ( ! mat->tex )
         {
           mat->tex = new ssgTexture ( mat->tfname ) ;
           if ( ! mat->tex )
             parser.error("cannot load texture: %s",mat->tfname) ;
         }
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
        sprintf( mat->name, "%s, sub#%d", mat_name, sub_index );
      else
        strcpy( mat->name, name );
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
      mat->trans = parser.parseFloat("trans");
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


static void parse_mesh( aseMesh* mesh )
{
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*TIMEVALUE"))
    {
      mesh -> time = parser.parseInt("time");
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
    else if (!strcmp(token,"*MESH_VERTEXNORMAL"))
    {
      if (mesh -> norms == 0)
        mesh -> norms = new sgVec3[ mesh -> num_verts ];
      u32 index = parser.parseInt("vertex #");
      if (index >= mesh -> num_verts)
        parser.error("bad vertex #");
      else
      {
        sgVec3& norm = mesh -> norms[ index ];
      
        norm[0] = parser.parseFloat("norm.x");
        norm[1] = parser.parseFloat("norm.y");
        norm[2] = parser.parseFloat("norm.z");
      }
    }
  }
}


static void add_mesh( aseMesh* mesh, aseMaterial* mat )
{
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
    return ;

  //allocate lists  
  u32 vcount = num_faces * 3 ;
  ssgVertexArray   *vlist = new ssgVertexArray ( vcount ) ;
  ssgTexCoordArray *tlist = 0 ;
  ssgColourArray   *clist = 0 ;
  ssgNormalArray   *nlist = 0 ;

  if ( mesh -> tverts )  
    tlist = new ssgTexCoordArray ( vcount ) ;
  if ( mesh -> cverts )
    clist = new ssgColourArray ( vcount ) ;
  if ( mesh -> norms )
    nlist = new ssgNormalArray ( vcount ) ;
  
  aseFace* face = mesh -> faces ;
  for ( u32 i=0; i<mesh -> num_faces; i++, face++ )
  {
    if ( mat->sub_flag && face->sub_index != mat->sub_index )
      continue ;

    for ( u32 j=0; j<3; j++ )
    {
      sgVec3 v ;
      sgCopyVec3 ( v, mesh -> verts[ face->v[j] ] ) ;
      vlist -> add ( v ) ;

      if ( mesh -> tverts )
      {
        sgVec2 tv ;
        sgCopyVec2 ( tv, mesh -> tverts[ face->tv[j] ] ) ;

        tv[0] *= mat->texrep[0] ;
        tv[1] *= mat->texrep[1] ;
        tv[0] += mat->texoff[0] ;
        tv[1] += mat->texoff[1] ;

        tlist -> add ( tv ) ;
      }

      if ( mesh -> cverts )
      {
        sgCopyVec3 ( v, mesh -> cverts[ face->cv[j] ] ) ;
        clist -> add ( v ) ;
      }

      if ( mesh -> norms )
      {
        sgCopyVec3 ( v, mesh -> norms[ face->v[j] ] ) ;
        nlist -> add ( v ) ;
      }
    }
  }
  
  ssgState *st = NULL ;
  if ( _ssgGetAppState != NULL && mat->tfname[0] != 0 )
    st =_ssgGetAppState ( mat->tfname ) ;
  if ( st == NULL )
    st = find_state ( mat ) ;

  char name[ 256 ];
  if ( mat->sub_flag )
    sprintf(name,"%s, sub#%d",mesh->name,mat->sub_index);
  else
    strcpy(name,mesh->name);
    
  ssgVtxTable *vtab = new ssgVtxTable ( GL_TRIANGLES,
    vlist, nlist, tlist, clist ) ; 
  vtab -> setCullFace ( TRUE );
  if ( st )
     vtab -> setState ( st ) ;
  vtab -> setName ( name ) ;
  current_branch -> addKid ( vtab ) ;
  
  //printf( "add_mesh: %s (%s)\n", name, mat -> tfname ) ;
}


static void parse_object()
{
  u32 mat_index = 0;

  aseMesh mesh;
  mesh.init();
  
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*NODE_NAME")) {
      if (parser.level==1) {
        char* name = parser.parseString("obj name");
        strcpy( mesh.name, name );
        //parser.message("mesh: %s",name);
      }
    }
    //Need: support for *NODE_PARENT
    //Need: support for *TM_ROW0, ...
#if 0
    else if (!strcmp(token,"*TM_POS")) {
      token = parser.parse_token("obj->pos.x");
      obj->pos.x = f32(atof(token));
      token = parser.parse_token("obj->pos.y");
      obj->pos.y = f32(atof(token));
      token = parser.parse_token("obj->pos.z");
      obj->pos.z = f32(atof(token));
    }
#endif
    else if (!strcmp(token,"*MESH"))
    {
      parse_mesh( &mesh );
    }
    else if (!strcmp(token,"*TM_ANIMATION")) {
      //parse_transform();
      int startLevel = parser.level;
      while ((token = parser.getLine( startLevel )) != NULL)
        ;
    }
    else if (!strcmp(token,"*MESH_ANIMATION")) {
      //parse_mesh_anim();
      int startLevel = parser.level;
      while ((token = parser.getLine( startLevel )) != NULL)
        ;
    }
    else if (!strcmp(token,"*MATERIAL_REF"))
    {
      mat_index = parser.parseInt("mat #");
    }
  }

#if 0
  {
    //add the transform
    sgMat4 tm ;
    sgMakeIdentMat4 ( tm ) ;
    ssgTransform *tr = new ssgTransform () ;
    tr -> setTransform ( tm ) ;
    current_branch -> addKid ( tr ) ;
  }
#endif

  //break apart the mesh for each sub material
  u32 num_subs = count_sub_materials ( mat_index );
  if ( num_subs == 0 )
    num_subs = 1 ;
  for ( u32 sub_index=0; sub_index<num_subs; sub_index++ )
  {
    aseMaterial* mat = find_material ( mat_index, sub_index );
    if ( mat )
       add_mesh ( &mesh, mat );
  }
  
  //NEED: to support mesh animations

  //free the local mesh
  mesh.free();
}


static bool parse()
{
  materials = new aseMaterial* [ MAX_MATERIALS ];
  states = new ssgSimpleState* [ MAX_STATES ];
  if ( !materials || !states )
  {
    parser.error("not enough memory");
    return false ;
  }

  num_materials = 0 ;
  num_states = 0 ;
  
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
    delete materials[ i ] ;
  delete[] materials ;
  materials = 0 ;

//Note: don't delete states because they are used in the scene graph
//  for ( i = 0 ; i < num_states ; i++ )
//    delete states[ i ] ;
  delete[] states ;
  states = 0 ;
}


ssgEntity *ssgLoadASE ( char *fname, ssgHookFunc hookfunc )
{
  current_branch = new ssgBranch ;
  parser.openFile( fname );
  if ( !parse() )
  {
     delete current_branch ;
     current_branch = 0 ;
  }
  parse_free();
  parser.closeFile();
  return current_branch ;
}

