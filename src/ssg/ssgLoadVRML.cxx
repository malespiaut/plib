
#include "ssgLocal.h"

#ifdef WIN32
#define strncasecmp strnicmp
#endif

#define VRML_MAX_TEXTURES 12    /* This *ought* to be enough! */

static FILE *loader_fd ;
//static FILE *VRMLlog;

static char *texture_fnames [ VRML_MAX_TEXTURES ] ;

static sgVec4 colors[12];
static sgVec3 xform[12];
static sgVec3 rot[12];

struct vecList
{
  sgVec3 pnt;
  vecList *next;
};

static vecList *VLIST[12];
static int VLISTverts[12];
static int VLISTcnt = 0;


struct texList
{
  sgVec2 pnt;
  texList *next;
};

static texList *TLIST[12];
static int TLISTverts[12];
static int TLISTcnt = 0;


struct nrmList
{
  sgVec3 nrm;
  nrmList *next;
};

static nrmList *NLIST[12];
static int NLISTnrms[12];
static int NLISTcnt = 0;

struct iList
{
  int index;
  iList *next;
};

static iList *ILIST[12];
static int ILISTn[12];
static int ILISTcnt = 0;


struct tiList
{
  int index;
  tiList *next;
};

static tiList *TILIST[12];
static int TILISTn[12];
static int TILISTcnt = 0;


struct niList
{
  int index;
  niList *next;
};

static niList *NILIST[12];
static int NILISTn[12];
static int NILISTcnt = 0;

struct _ssgMaterial
{
  sgVec4 spec ;
  sgVec4 emis ;
  sgVec4 rgb  ;
  float  shi  ;
} ;

static int num_materials = 0 ;
static int num_states    = 0 ;
static int num_textures  = 0 ;
static sgVec3 *vtab = NULL ;

static ssgHookFunc      current_hookFunc = NULL ;
static _ssgMaterial    *current_material = NULL ;
static sgVec4          *current_colour   = NULL ;
static ssgTexture      *current_texture  = NULL ;
static ssgBranch       *current_branch   = NULL ;
static char            *current_tfname   = NULL ;
static char            *current_data     = NULL ;
static char             current_name[32];

//static _ssgMaterial   *mlist    [ VRML_MAX_TEXTURES ] ;
//static ssgSimpleState *slist    [ VRML_MAX_TEXTURES ] ;
//static sgVec4         *clist    [ VRML_MAX_TEXTURES ] ;
static ssgTexture       *tex_list [ VRML_MAX_TEXTURES ] ;
static int               texCnt = 0;
//static sgMat4 current_matrix ;
static sgVec2 texrep ;


int VRMLdo_material ( char *s ) ;
int VRMLdo_object   ( char *s ) ;
int VRMLdo_name     ( char *s ) ;
int VRMLdo_data     ( char *s ) ;
int VRMLdo_texture  ( char *s ) ;
int VRMLdo_texrep   ( char *s ) ;
int VRMLdo_rot      ( char *s ) ;
int VRMLdo_loc      ( char *s ) ;
int VRMLdo_url      ( char *s ) ;
int VRMLdo_numvert  ( char *s ) ;
int VRMLdo_numsurf  ( char *s ) ;
int VRMLdo_surf     ( char *s ) ;
int VRMLdo_mat      ( char *s ) ;
int VRMLdo_refs     ( char *s ) ;


//chris
int VRMLdo_diffuseColor    ( char *s );
int VRMLdo_def        ( char *s );
int VRMLdo_indexedFaceSet  ( char *s );
int VRMLdo_shape      ( char *s );
int VRMLdo_viewPoint    ( char *s );
int VRMLdo_transform    ( char *s );
int VRMLdo_child      ( char *s );
int VRMLdo_geom        ( char *s );
int VRMLdo_points      ( char *s );
int VRMLdo_coord      ( char *s );
int VRMLdo_coordIndex    ( char *s );
int VRMLdo_normal      ( char *s );
int VRMLdo_normalIndex    ( char *s );
int VRMLdo_translation    ( char *s );
int VRMLdo_rotation      ( char *s );
int VRMLdo_texCoordIndex  ( char *s );
int VRMLdo_texCoord      ( char *s );

//viewpoint
int VRMLdo_position    ( char *s );
int VRMLdo_orientation  ( char *s );
int VRMLdo_fov      ( char *s );
int VRMLdo_description  ( char *s );

int VRMLdo_obj_world ( char *s ) ;
int VRMLdo_obj_poly  ( char *s ) ;
int VRMLdo_obj_group ( char *s ) ;
int VRMLdo_obj_light ( char *s ) ;

#define PARSE_CONT   0
#define PARSE_POP    1


struct Tag
{
  char *token ;
  int (*func) ( char *s ) ;
} ;

void VRMLskip_spaces ( char **s )
{
  while ( **s == ' ' || **s == '\t' )
    (*s)++ ;
}

void find_point ( char ** s)
{
  while ( **s != 'p' )
    (*s)++;
}

void next_bracket (char **s)
{
  while ( **s != '{' && **s != '}')
  {
    (*s)++;
  }
}

void next_Sqbracket (char **s)
{
  while ( **s != '[' && **s != ']')
  {
    (*s)++;
  }
}
void VRMLskip_quotes ( char **s )
{
  VRMLskip_spaces ( s ) ;
  
  if ( **s == '\"' )
  {
    (*s)++ ;
    
    char *t = *s ;
    
    while ( *t != '\0' && *t != '\"' )
      t++ ;
    
    if ( *t != '\"' )
      fprintf ( stderr, "ac_to_gl: Mismatched double-quote ('\"') in '%s'\n", *s ) ;
    
    *t = '\0' ;
  }
  else
    fprintf ( stderr, "ac_to_gl: Expected double-quote ('\"') in '%s'\n", *s ) ;
}



int VRMLsearch ( Tag *tags, char *s )
{
  VRMLskip_spaces ( & s ) ;
  
  for ( int i = 0 ; tags[i].token != NULL ; i++ )
    if ( strncasecmp ( tags[i].token, s, strlen(tags[i].token) ) == 0 )
    {
      s += strlen ( tags[i].token ) ;
      
      VRMLskip_spaces ( & s ) ;
      
      return (*(tags[i].func))( s ) ;
    }
    
    //fprintf ( stderr, "ac_to_gl: Unrecognised token '%s'\n", s ) ;
    //exit ( 1 ) ;
    
    return 0 ;  /* Should never get here */
}

// normalIndex must come before normal
// same with coord and coordIndex
Tag VRMLtop_tags [] =
{
  //{"DEF"        ,  VRMLdo_def        },
  {"texture",VRMLdo_texture},
  {"texCoordIndex",VRMLdo_texCoordIndex},
  {"texCoord",VRMLdo_texCoord},
  {"translation",VRMLdo_translation},
  {"rotation",VRMLdo_rotation},
  {"diffuseColor",VRMLdo_diffuseColor},
  {"point",VRMLdo_points},
  {"normalIndex",VRMLdo_normalIndex},
  {"normal ",VRMLdo_normal},
  {"IndexedFaceSet"  ,  VRMLdo_indexedFaceSet  },
  {"viewPoint"    ,  VRMLdo_viewPoint    },
  
} ;


Tag VRMLobject_tags [] =
{
  {"coordIndex",VRMLdo_coordIndex},
  {"point",VRMLdo_points},
  {"coord",VRMLdo_coord},
  {"normalIndex",VRMLdo_normalIndex},
  {"normal ",VRMLdo_normal},
  {"IndexedFaceSet"  ,  VRMLdo_indexedFaceSet  },
  {"viewPoint"    ,  VRMLdo_viewPoint    },
  {"Transform"    ,  VRMLdo_transform    },
  {"Children"      ,  VRMLdo_child      },
  {"shape"      ,  VRMLdo_shape      },
  {"geometry"      ,  VRMLdo_geom      },
  
  { NULL , NULL },
}; 

Tag viewPoint_tags [] =
{
  {"position"      ,  VRMLdo_position      },
  {"orientation"    ,  VRMLdo_orientation    },
  {"fieldOfView"    ,  VRMLdo_fov        },
  {"description"    ,  VRMLdo_description    },
  {NULL,NULL},
};


Tag point_tag [] =
{
  {"point",VRMLdo_points},
  {NULL,NULL},
};

#define OBJ_WORLD  0
#define OBJ_POLY   1
#define OBJ_GROUP  2
#define OBJ_LIGHT  3

int VRMLdo_obj_world ( char * ) { return OBJ_WORLD ; } 
int VRMLdo_obj_poly  ( char * ) { return OBJ_POLY  ; }
int VRMLdo_obj_group ( char * ) { return OBJ_GROUP ; }
int VRMLdo_obj_light ( char * ) { return OBJ_LIGHT ; }

//int last_num_kids    = -1 ;
//static int VRMLcurrent_flags    = -1 ;

//static ssgSimpleState *find_state ( _ssgMaterial *mat,
//                   ssgTexture *tex, char * /*tfname*/ )
//{
//  for ( int i = 0 ; i < num_states ; i++ )
//  {
//    ssgSimpleState *st2 = slist [ i ] ;
//    
//    if ( tex == NULL && st2->isEnabled ( GL_TEXTURE_2D ) )
//      continue ;
//    
//    if ( tex != NULL && ! st2->isEnabled ( GL_TEXTURE_2D ) )
//      continue ;
//    
//    if ( tex != NULL && tex -> getHandle() != st2 -> getTextureHandle () )
//      continue ;
//    
//    if ( ! sgEqualVec4 ( mat->emis, st2->getMaterial ( GL_EMISSION ) ) ||
//      ! sgEqualVec4 ( mat->spec, st2->getMaterial ( GL_SPECULAR ) ) ||
//      (int)( mat->rgb[3] < 0.99 ) != st2 -> isTranslucent () ||
//      mat -> shi != st2->getShininess () )
//      continue ;
//    
//    return st2 ;
//  }
//  
//  ssgSimpleState *st = new ssgSimpleState () ;
//  slist [ num_states++ ] = st ;
//  if ( tex != NULL )
//  {
//    st -> setTexture ( tex ) ;
//    st -> enable     ( GL_TEXTURE_2D ) ;
//  }
//  else
//    st -> disable    ( GL_TEXTURE_2D ) ;
//  st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
//  st -> setMaterial ( GL_EMISSION, mat -> emis ) ;
//  st -> setShininess ( mat -> shi ) ;
//  st -> enable ( GL_COLOR_MATERIAL ) ;
//  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
//  st -> enable  ( GL_LIGHTING       ) ;
//  st -> setShadeModel ( GL_SMOOTH ) ;
//  if ( mat -> rgb[3] < 0.99 )
//  {
//    st -> disable ( GL_ALPHA_TEST ) ;
//    st -> enable  ( GL_BLEND ) ;
//    st -> setTranslucent () ;
//  }
//  else
//  {
//    st -> disable ( GL_BLEND ) ;
//    st -> setOpaque () ;
//  }
//  return st ;
//}


int VRMLdo_material ( char *s )
{
  
  return PARSE_CONT ;
}

int VRMLdo_diffuseColor ( char *s )
{
  
  char x[32],y[32],z[32];
  sscanf(s,"%s %s %s",x,y,z);
  //fprintf(VRMLlog,"\nfound diffuse color: %s %s %s\n",x,y,z);
  sgSetVec4(colors[VLISTcnt],atof(x),atof(y),atof(z),1.0f);
  //clist[VLISTcnt] = &current_colour[0];
  return 0;
}



int VRMLdo_object   ( char *  /* s */ )
{
  return PARSE_CONT ;
}


int VRMLdo_name ( char *s )
{
  VRMLskip_quotes ( &s ) ;
  current_branch -> setName ( s ) ;
  return PARSE_CONT ;
}


int VRMLdo_data     ( char *s )
{
  int len = strtol ( s, NULL, 0 ) ;
  
  current_data = new char [ len + 1 ] ;
  
  for ( int i = 0 ; i < len ; i++ )
    current_data [ i ] = fgetc ( loader_fd ) ;
  
  current_data [ len ] = '\0' ;
  
  fgetc ( loader_fd ) ;  /* Final RETURN */
  
  if ( current_hookFunc != NULL )
  {
    ssgBranch *br = (*current_hookFunc) ( current_data ) ;
    
    if ( br != NULL )
    {
      current_branch -> addKid ( br ) ;
      current_branch = br ;
    }
    
    current_data = NULL ;
  }
  
  return PARSE_CONT ;
}


int VRMLdo_translation( char *s ){
  
  char a[32],b[32],c[32];
  int result;
  //sgVec3 tmpVec;
  result = sscanf(s,"%s %s %s",a,b,c);
  //fprintf(VRMLlog,"\nfound transform: %d   %s %s %s",result,a,b,c);
  sgSetVec3(xform[VLISTcnt],atof(a),atof(b),atof(c));
  //xform[VLISTcnt]->setTransform(tmpVec);
  return PARSE_CONT;
}

int VRMLdo_rotation(char *s){
/*
char a[32],b[32],c[32];
int result;
//sgVec3 tmpVec;
result = sscanf(s,"%s %s %s",a,b,c);
//fprintf(VRMLlog,"\nfound rotation: %d   %s %s %s",result,a,b,c);
sgSetVec3(rot[VLISTcnt],atof(a),atof(b),atof(c));
  */
  return PARSE_CONT;
}

int VRMLdo_points(char *s)
{
  //fprintf(VRMLlog,"\n found points.... \n");
  return 0;
}


///////////////////////////////////////////////////
///          Read Points                  //
///////////////////////////////////////////////////
int VRMLdo_coord(char *s)
{
  next_Sqbracket(&s);
  char bracket[] = "]";
  char bracket2[]= "}";
  char tmp[1024];
  int result;
  float x,y,z;
  
  char d[15][32];
  vecList *vecListRoot, *vecListCurrent, *oldvecList;
  vecListRoot = new vecList;
  vecListCurrent = vecListRoot;
  vecListRoot->next = vecListRoot;
  int vecListcnt = 0;
  
  //fprintf(VRMLlog,"reading points...");
  while (1) {
    fgets(tmp,1024,loader_fd);
    result = sscanf(tmp,"%s %s %s    %s %s %s   %s %s %s   %s %s %s   %s %s %s",d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8],d[9],d[10],d[11],d[12],d[13],d[14]);
    if (result == EOF) {
      //break;
      //fprintf(VRMLlog,"\nREACHED EOF");
      return 1;
    }
    if (result < 3) {
      //fprintf(VRMLlog,"\nmust have found a bracket...%d",result);
      
      vecListCurrent = vecListRoot;
      
      VLIST[VLISTcnt]=vecListRoot;
      VLISTverts[VLISTcnt]=vecListcnt;
      VLISTcnt++;
      //final check
      vecListCurrent = vecListRoot;
      /*for(int i=0;i<VLISTverts[VLISTcnt-1];i++)
      {
      //fprintf(VRMLlog,"final:x%f y%f z%f",vecListCurrent->pnt[0],vecListCurrent->pnt[1],vecListCurrent->pnt[2]);
      vecListCurrent = vecListCurrent->next;
      }
      */
      break;
      return 1;
    }
    for (int i = 0; i < result ; i++){
      if (strcmp(bracket,d[i]) == 0 && strcmp(bracket2,d[i])) {
        //break; 
        //break; 
        // in here must read data before bracket
        //fprintf(VRMLlog,"\nFOUND CLOSING BRACKET result: %d\n",result);
        break;
        return 1;
      }
    }
    //fprintf(VRMLlog,"\nRead %d items  ", result);
    //fprintf(VRMLlog,"Count: %d   x: %s   y: %s    z: %s ", vecListcnt,d[0],d[1],d[2]);
    x = atof(d[0]); y = atof(d[1]); z = atof(d[2]);
    //fprintf(VRMLlog,"\nconversion: %f %f %f",x,y,z);
    sgSetVec3(vecListCurrent->pnt,x,y,z);
    //fprintf(VRMLlog,"\nsgVec3: %f %f %f",vecListCurrent->pnt[0],vecListCurrent->pnt[1],vecListCurrent->pnt[2]);
    oldvecList = vecListCurrent;
    vecListCurrent = new vecList;
    oldvecList->next = vecListCurrent;
    vecListcnt++;
    if (result > 3) {
      //fprintf(VRMLlog,"val345: %f %f %f",atof(d[3]),atof(d[4]),atof(d[5]));
      sgSetVec3(vecListCurrent->pnt,atof(d[3]),atof(d[4]),atof(d[5]));
      oldvecList = vecListCurrent;
      vecListCurrent = new vecList;
      oldvecList->next = vecListCurrent;
      vecListcnt++;
    }
    if (result > 6) {
      sgSetVec3(vecListCurrent->pnt,atof(d[6]),atof(d[7]),atof(d[8]));
      oldvecList = vecListCurrent;
      vecListCurrent = new vecList;
      oldvecList->next = vecListCurrent;
      vecListcnt++;
    }
    if (result > 9) {
      sgSetVec3(vecListCurrent->pnt,atof(d[9]),atof(d[10]),atof(d[11]));
      oldvecList = vecListCurrent;
      vecListCurrent = new vecList;
      oldvecList->next = vecListCurrent;
      vecListcnt++;
    }
    if (result > 12) {
      sgSetVec3(vecListCurrent->pnt,atof(d[12]),atof(d[13]),atof(d[14]));
      oldvecList = vecListCurrent;
      vecListCurrent = new vecList;
      oldvecList->next = vecListCurrent;
      vecListcnt++;
    }
  }
  
  //does not get here
  return 0 ;
}




int VRMLdo_texCoord(char *s)
{
  next_Sqbracket(&s);
  char bracket[] = "]";
  char bracket2[]= "}";
  char tmp[1024];
  int result;
  float u,v;
  
  char d[15][32];
  texList *texListRoot, *texListCurrent, *oldtexList;
  texListRoot = new texList;
  texListCurrent = texListRoot;
  texListRoot->next = texListRoot;
  int texListcnt = 0;
  
  //fprintf(VRMLlog,"reading TEXpoints...");
  while (1) {
    fgets(tmp,1024,loader_fd);
    result = sscanf(tmp,"%s %s    %s %s    %s %s    %s %s    %s %s   %s %s    %s %s",d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8],d[9],d[10],d[11],d[12],d[13] /*,d[14]*/ );
    if (result == EOF) {
      //break;
      //fprintf(VRMLlog,"\nREACHED EOF");
      return 1;
    }
    if (result < 2) {
      //fprintf(VRMLlog,"\nmust have found a bracket...%d",result);
      
      texListCurrent = texListRoot;
      
      TLIST[TLISTcnt]=texListRoot;
      TLISTverts[TLISTcnt]=texListcnt;
      TLISTcnt++;
      //final check
      texListCurrent = texListRoot;
      /*for(int i=0;i<VLISTverts[VLISTcnt-1];i++)
      {
      //fprintf(VRMLlog,"final:x%f y%f z%f",vecListCurrent->pnt[0],vecListCurrent->pnt[1],vecListCurrent->pnt[2]);
      vecListCurrent = vecListCurrent->next;
      }
      */
      break;
      return 1;
    }
    for (int i = 0; i < result ; i++){
      if (strcmp(bracket,d[i]) == 0 && strcmp(bracket2,d[i])) {
        //break; 
        //break; 
        // in here must read data before bracket
        //fprintf(VRMLlog,"\nFOUND CLOSING BRACKET result: %d\n",result);
        break;
        return 1;
      }
    }
    //fprintf(VRMLlog,"\nRead %d items  ", result);
    //fprintf(VRMLlog,"Count: %d   x: %s   y: %s    z: %s ", texListcnt,d[0],d[1]);
    u = atof(d[0]); v = atof(d[1]); 
    //fprintf(VRMLlog,"\nRESULT: %d", result);
    //fprintf(VRMLlog,"\n texCoord %d 0,1: %f %f ",texListcnt,u,v);

    sgSetVec2(texListCurrent->pnt,u,v);
    //fprintf(VRMLlog,"\nsgVec3: %f %f %f",vecListCurrent->pnt[0],vecListCurrent->pnt[1],vecListCurrent->pnt[2]);
    oldtexList = texListCurrent;
    texListCurrent = new texList;
    oldtexList->next = texListCurrent;
    texListcnt++;
    
    if (result > 2) {
      //fprintf(VRMLlog,"\ntexCoord %d  3,4: %f %f ",texListcnt,atof(d[2]),atof(d[3]));
      sgSetVec2(texListCurrent->pnt,atof(d[2]),atof(d[3]));
      oldtexList = texListCurrent;
      texListCurrent = new texList;
      oldtexList->next = texListCurrent;
      texListcnt++;
    }
    if (result > 4) {
    //fprintf(VRMLlog,"\ntexCoord %d 5,6: %f %f ",texListcnt,atof(d[4]),atof(d[5]));
      sgSetVec2(texListCurrent->pnt,atof(d[4]),atof(d[5]));
      oldtexList = texListCurrent;
      texListCurrent = new texList;
      oldtexList->next = texListCurrent;
      texListcnt++;
    }
    if (result > 6) {
    //fprintf(VRMLlog,"\ntexCoord %d 7,8: %f %f ",texListcnt,atof(d[6]),atof(d[7]));
      sgSetVec2(texListCurrent->pnt,atof(d[6]),atof(d[7]));
      oldtexList = texListCurrent;
      texListCurrent = new texList;
      oldtexList->next = texListCurrent;
      texListcnt++;
    }
    if (result > 8) {
      //fprintf(VRMLlog,"\ntexCoord %d 9,10: %f %f ",texListcnt,atof(d[8]),atof(d[9]));
      sgSetVec2(texListCurrent->pnt,atof(d[8]),atof(d[9]));
      oldtexList = texListCurrent;
      texListCurrent = new texList;
      oldtexList->next = texListCurrent;
      texListcnt++;
    }
    if (result > 10) {
      //fprintf(VRMLlog,"\ntexCoord %d 11,12: %f %f ",texListcnt,atof(d[10]),atof(d[11]));
      sgSetVec2(texListCurrent->pnt,atof(d[10]),atof(d[11]));
      oldtexList = texListCurrent;
      texListCurrent = new texList;
      oldtexList->next = texListCurrent;
      texListcnt++;
    }
    if (result > 12) {
      //fprintf(VRMLlog,"\ntexCoord %d 11,12: %f %f ",texListcnt,atof(d[12]),atof(d[13]));
      sgSetVec2(texListCurrent->pnt,atof(d[12]),atof(d[13]));
      oldtexList = texListCurrent;
      texListCurrent = new texList;
      oldtexList->next = texListCurrent;
      texListcnt++;
    }
  }

  
  //does not get here
  return 0 ;
}


int VRMLdo_normalIndex (char *s)
{
  next_Sqbracket(&s);
  char bracket[] = "]";
  char bracket2[]= "}";
  char tmp[1024];
  int result;
  
  char d[21][32];
  niList *niListRoot, *niListCurrent, *niListOld;
  niListRoot = new niList;
  niListCurrent = niListRoot;
  niListRoot->next = niListRoot;
  int niListcnt = 0; //local number of indices in list
  
  //fprintf(VRMLlog,"\nreading normalIndices...");
  while (1) {
    fgets(tmp,1024,loader_fd);
    
    result = sscanf(tmp,"%s %s %s    %s %s %s   %s %s %s   %s %s %s  %s %s %s   %s %s %s  %s %s %s",d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8],d[9],d[10],d[11],d[12],d[13],d[14],d[15],d[16],d[17],d[18],d[19],d[20]);
    if (result == EOF) {
      //break;
      //fprintf(VRMLlog,"\nREACHED EOF");
      return 1;
    }
    if (result < 3) {
      //fprintf(VRMLlog,"\nmust have found a bracket...%d",result);
      
      niListCurrent = niListRoot;
      
      NILIST[NILISTcnt] = niListRoot;
      NILISTn[NILISTcnt]= niListcnt;
      NILISTcnt++;
      break;
      return 1;
    }
    int i = 0 ;

    for (i = 0; i < result ; i++){
      if (strcmp(bracket,d[i]) == 0 && strcmp(bracket2,d[i])) {
        //break; 
        // in here must read data before bracket
        //fprintf(VRMLlog,"\nFOUND CLOSING BRACKET result: %d\n",result);
        break;
        return 1;
      }
    }
    //fprintf(VRMLlog,"\nRead %d items  ", result);
    //fprintf(VRMLlog,"Count: %d   x: %s   y: %s    z: %s ", niListcnt,d[0],d[1],d[2]);
    for (i=0;i < result ; i++){
      if (atoi(d[i]) >= 0) { // test for -1, VRML's end of a triangle marker
        niListOld = niListCurrent;
        niListCurrent->index=(atoi(d[i]));
        niListCurrent = new niList;
        niListOld->next = niListCurrent;
        niListcnt++;
      }
    }
  }
  return 0 ;
}  


int VRMLdo_normal(char *s)
{
  next_Sqbracket(&s);
  char bracket[] = "]";
  char bracket2[]= "}";
  char tmp[1024];
  int result;
  float x,y,z;
  char * str_result;
  char d[15][32];
  nrmList *nrmListRoot, *nrmListCurrent, *oldnrmList;
  nrmListRoot = new nrmList;
  nrmListCurrent = nrmListRoot;
  nrmListRoot->next = nrmListRoot;
  int nrmListcnt = 0;
  
  //fprintf(VRMLlog,"reading NORMALS...");
  while (1) {
    fgets(tmp,1024,loader_fd);
    
    result = sscanf(tmp,"%s %s %s    %s %s %s   %s %s %s   %s %s %s   %s %s %s",d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8],d[9],d[10],d[11],d[12],d[13],d[14]);
    if (result == EOF) {
      //break;
      //fprintf(VRMLlog,"\nREACHED EOF");
      return 1;
    }
    if (result < 3) {
      //fprintf(VRMLlog,"\nmust have found a bracket...%d",result);
      
      nrmListCurrent = nrmListRoot;
      
      NLIST[NLISTcnt]=nrmListRoot;
      NLISTnrms[NLISTcnt]=nrmListcnt;
      NLISTcnt++;
      //final check
      nrmListCurrent = nrmListRoot;
      for(int i=0;i<NLISTnrms[NLISTcnt-1];i++)
      {
        //fprintf(VRMLlog,"final:x%f y%f z%f",nrmListCurrent->nrm[0],nrmListCurrent->nrm[1],nrmListCurrent->nrm[2]);
        nrmListCurrent = nrmListCurrent->next;
      }
      break;
      return 1;
    }
    for (int i = 0; i < result ; i++){
      if (strcmp(bracket,d[i]) == 0 && strcmp(bracket2,d[i])) {
        //break;  
        // in here must read data before bracket
        //fprintf(VRMLlog,"\nFOUND CLOSING BRACKET result: %d\n",result);
        break;
        return 1;
      }
    }
    
    //fprintf(VRMLlog,"\nRead %d items  ", result);
    //fprintf(VRMLlog,"Count: %d   x: %s   y: %s    z: %s ", nrmListcnt,d[0],d[1],d[2]);
    x = atof(d[0]); y = atof(d[1]); z = atof(d[2]);
    //fprintf(VRMLlog,"\nconversion: %f %f %f",x,y,z);
    sgSetVec3(nrmListCurrent->nrm,x,y,z);
    //fprintf(VRMLlog,"\nsgnrm3: %f %f %f",nrmListCurrent->nrm[0],nrmListCurrent->nrm[1],nrmListCurrent->nrm[2]);
    oldnrmList = nrmListCurrent;
    nrmListCurrent = new nrmList;
    oldnrmList->next = nrmListCurrent;
    nrmListcnt++;
    if (result > 3) {
      //fprintf(VRMLlog,"val345: %f %f %f",atof(d[3]),atof(d[4]),atof(d[5]));
      sgSetVec3(nrmListCurrent->nrm,atof(d[3]),atof(d[4]),atof(d[5]));
      oldnrmList = nrmListCurrent;
      nrmListCurrent = new nrmList;
      oldnrmList->next = nrmListCurrent;
      nrmListcnt++;
    }
    if (result > 6) {
      //fprintf(VRMLlog,"val345: %f %f %f",atof(d[6]),atof(d[7]),atof(d[8]));
      sgSetVec3(nrmListCurrent->nrm,atof(d[6]),atof(d[7]),atof(d[8]));
      oldnrmList = nrmListCurrent;
      nrmListCurrent = new nrmList;
      oldnrmList->next = nrmListCurrent;
      nrmListcnt++;
    }
    if (result > 9) {
      //fprintf(VRMLlog,"val345: %f %f %f",atof(d[9]),atof(d[10]),atof(d[11]));
      sgSetVec3(nrmListCurrent->nrm,atof(d[9]),atof(d[10]),atof(d[11]));
      oldnrmList = nrmListCurrent;
      nrmListCurrent = new nrmList;
      oldnrmList->next = nrmListCurrent;
      nrmListcnt++;
    }
    if (result > 12) {
      //fprintf(VRMLlog,"val345: %f %f %f",atof(d[12]),atof(d[13]),atof(d[14]));
      sgSetVec3(nrmListCurrent->nrm,atof(d[12]),atof(d[13]),atof(d[14]));
      oldnrmList = nrmListCurrent;
      nrmListCurrent = new nrmList;
      oldnrmList->next = nrmListCurrent;
      nrmListcnt++;
    }
    // check for last character of last string to be a bracket
    str_result = strchr(s,'}');
    if (str_result != NULL) { // found bracket
      //fprintf(VRMLlog,"\nBracket at end of string");
      break;
      return 1;
    }
    str_result = strchr(s,']');
    if (str_result != NULL) { // found bracket
      //fprintf(VRMLlog,"\nBracket at end of string");
      break;
      return 1;
    }
  }
  
  //does not get here
  return 0 ;
}


int VRMLdo_coordIndex(char *s)
{
  next_Sqbracket(&s);
  char bracket[] = "]";
  char bracket2[]= "}";
  char tmp[1024];
  int result;
  char *str_result;
  
  char d[21][32];
  iList *iListRoot, *iListCurrent, *iListOld;
  iListRoot = new iList;
  iListCurrent = iListRoot;
  iListRoot->next = iListRoot;
  int iListcnt = 0; //local number of indices in list
  
  //fprintf(VRMLlog,"\nreading indices...");
  int QUIT = 0;
  while (1) {
    fgets(tmp,1024,loader_fd);
    
    result = sscanf(tmp,"%s %s %s    %s %s %s   %s %s %s   %s %s %s  %s %s %s   %s %s %s  %s %s %s",d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8],d[9],d[10],d[11],d[12],d[13],d[14],d[15],d[16],d[17],d[18],d[19],d[20]);
    if (result == EOF) {
      //break;
      //fprintf(VRMLlog,"\nREACHED EOF");
      return 1;
    }
    // check for last character of last string to be a bracket
    str_result = strchr(tmp,'}');
    if (str_result != NULL) { // found bracket
      //fprintf(VRMLlog,"\nBracket at end of string");
      QUIT = 1;
      //break;
      //return 1;
    }
    str_result = strchr(tmp,']');
    if (str_result != NULL) { // found bracket
      //fprintf(VRMLlog,"\nBracket at end of string");
      QUIT = 1;
      //break;
      //return 1;
    }
    int i=0;
    for (i = 0; i < result ; i++){
      if (strcmp(bracket,d[i]) == 0 && strcmp(bracket2,d[i])) {
        //break; 
        // in here must read data before bracket
        //fprintf(VRMLlog,"\nFOUND CLOSING BRACKET result: %d\n",result);
        break;
        return 0;
      }
    }
    if (result < 3 && QUIT == 1) {
      //fprintf(VRMLlog,"\nmust have found a bracket...%d",result);
      
      iListCurrent = iListRoot;
      
      ILIST[ILISTcnt] = iListRoot;
      ILISTn[ILISTcnt]=iListcnt;
      ILISTcnt++;
      break;
      return 0;
    }
    if ( result > 3 && QUIT == 1){
      for (i=0;i < result ; i++){
        if (atoi(d[i]) >= 0) { // test for -1, VRML's end of a triangle marker
          iListOld = iListCurrent;
          iListCurrent->index=(atoi(d[i]));
          iListCurrent = new iList;
          iListOld->next = iListCurrent;
          iListcnt++;
        }
      }
      iListCurrent = iListRoot;
      
      ILIST[ILISTcnt] = iListRoot;
      ILISTn[ILISTcnt]=iListcnt;
      ILISTcnt++;
      break;
      return 0;
    }
    //  fprintf(VRMLlog,"\nRead %d items  ", result);
    //fprintf(VRMLlog,"Count: %d   x: %s   y: %s    z: %s ", iListcnt,d[0],d[1],d[2]);
    for (i=0;i < result ; i++){
      if (atoi(d[i]) >= 0) { // test for -1, VRML's end of a triangle marker
        iListOld = iListCurrent;
        iListCurrent->index=(atoi(d[i]));
        iListCurrent = new iList;
        iListOld->next = iListCurrent;
        iListcnt++;
      }
    }
  }
  return 0 ;
}  


int VRMLdo_texCoordIndex(char *s)
{
  next_Sqbracket(&s);
  char bracket[] = "]";
  char bracket2[]= "}";
  char tmp[1024];
  int result;
  char *str_result;
  
  char d[21][32];
  tiList *tiListRoot, *tiListCurrent, *tiListOld;
  tiListRoot = new tiList;
  tiListCurrent = tiListRoot;
  tiListRoot->next = tiListRoot;
  int tiListcnt = 0; //local number of indices in list
  
  //fprintf(VRMLlog,"\nreading TEXindices...");
  int QUIT = 0;
  while (1) {
    fgets(tmp,1024,loader_fd);
    
    result = sscanf(tmp,"%s %s %s    %s %s %s   %s %s %s   %s %s %s  %s %s %s   %s %s %s  %s %s %s",d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8],d[9],d[10],d[11],d[12],d[13],d[14],d[15],d[16],d[17],d[18],d[19],d[20]);
    if (result == EOF) {
      //break;
      //fprintf(VRMLlog,"\nREACHED EOF");
      return 1;
    }
    // check for last character of last string to be a bracket
    str_result = strchr(tmp,'}');
    if (str_result != NULL) { // found bracket
      //fprintf(VRMLlog,"\nTEX__Bracket at end of string");
      QUIT = 1;
      //break;
      //return 1;
    }
    str_result = strchr(tmp,']');
    if (str_result != NULL) { // found bracket
      //fprintf(VRMLlog,"\nTEX___Bracket at end of string");
      QUIT = 1;
      //break;
      //return 1;
    }
    int i = 0 ;

    for (i = 0; i < result ; i++){
      if (strcmp(bracket,d[i]) == 0 && strcmp(bracket2,d[i])) {
        //break; 
        // in here must read data before bracket
        //fprintf(VRMLlog,"\nTEX___FOUND CLOSING BRACKET result: %d\n",result);
        break;
        return 0;
      }
    }
    if (result < 3 && QUIT == 1) {
      //fprintf(VRMLlog,"\nTEXmust have found a bracket...%d",result);
      
      tiListCurrent = tiListRoot;
      
      TILIST[TILISTcnt] = tiListRoot;
      TILISTn[ILISTcnt]=tiListcnt;
      TILISTcnt++;
      break;
      return 0;
    }
    if ( result > 3 && QUIT == 1){
      for (i=0;i < result ; i++){
        if (atoi(d[i]) >= 0) { // test for -1, VRML's end of a triangle marker
          tiListOld = tiListCurrent;
          tiListCurrent->index=(atoi(d[i]));
          tiListCurrent = new tiList;
          tiListOld->next = tiListCurrent;
          tiListcnt++;
        }
      }
      tiListCurrent = tiListRoot;
      
      TILIST[TILISTcnt] = tiListRoot;
      TILISTn[TILISTcnt]= tiListcnt;
      TILISTcnt++;
      break;
      return 0;
    }
      //fprintf(VRMLlog,"\nTEXRead %d items  ", result);
    //fprintf(VRMLlog,"TEXCount: %d   x: %s   y: %s    z: %s ", tiListcnt,d[0],d[1]);
    for (i=0;i < result ; i++){
      if (atoi(d[i]) >= 0) { // test for -1, VRML's end of a triangle marker
        tiListOld = tiListCurrent;
        tiListCurrent->index=(atoi(d[i]));
        tiListCurrent = new tiList;
        tiListOld->next = tiListCurrent;
        tiListcnt++;
      }
    }
  }
  return 0 ;
}  




int VRMLdo_def        ( char *s )
{
  char name [ 32 ];
  sscanf(s,"%s",name);
  VRMLskip_spaces(&s);
  //fprintf(VRMLlog,"\nfound DEF %s",name);
  strcpy(current_name,name);
  //VRMLsearch(VRMLobject_tags,s);
  return 0;
}


int VRMLdo_indexedFaceSet  ( char *s )
{
  return 0;
}


int VRMLdo_shape      ( char *s )
{
  return 0;
}

int VRMLdo_texture      ( char *s )
{
  
  char tmp[1024];
  char dummy[16],filename[256];
  char *t = filename;
  fgets(tmp,1024,loader_fd);
  sscanf(tmp,"%s %s",dummy,filename);
  //fprintf(VRMLlog,"\nfound texture %s\n",filename);
  VRMLskip_quotes(&t);

  tex_list[texCnt] = new ssgTexture (t);
  //fprintf(VRMLlog,"\nMADE texture %s\n",t);
  texCnt++;

  return 0;
}

/////////////////////////////////////////////////////
// GEOMETRY
/////////////////////////////////////////////////////
int VRMLdo_geom      ( char *s )
{  
  return 0;
}

int VRMLdo_viewPoint    ( char *s )
{
  char dummy_str[32];
  //fprintf(VRMLlog,"\nfound viewpoint");
  next_bracket(&s);
  VRMLsearch(viewPoint_tags,s);
  sscanf(s,"%s",dummy_str);
  return 0;
}
int VRMLdo_transform    ( char *s )
{
  return 0;
}
int VRMLdo_child      ( char *s )
{
  return 0;
}

//viewpoint

int VRMLdo_position    ( char *s )
{
  return 0;
}

int VRMLdo_orientation  ( char *s )
{
  return 0;
}

int VRMLdo_fov      ( char *s )
{
  return 0;
}

int VRMLdo_description  ( char *s )
{
  return 0;
}





ssgEntity *ssgLoadVRML ( char *fname, ssgHookFunc hookfunc)// int &leafCount, vTableList &vTableList; )
{
  current_hookFunc = hookfunc ;
  char filename [ 1024 ] ;
  if ( fname [ 0 ] != '/' && _ssgModelPath != NULL && 
                             _ssgModelPath [ 0 ] != '\0' )
  {
    strcpy ( filename, _ssgModelPath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, fname ) ;
  }
  else
    strcpy ( filename, fname ) ;
  
  //init rots and xforms
  for (int e=0;e<12;e++){
    sgZeroVec3(rot[e]);
    sgZeroVec3(xform[e]);
  }
  
  num_textures  = 0 ;
  num_materials = 0 ;
  num_states    = 0 ;
  vtab = NULL ;
  //leafCount = 0;
  
  current_material = NULL ;
  current_colour   = NULL ;
  current_texture  = NULL ;
  current_tfname   = NULL ;
  current_branch   = NULL ;
  //sgVec4 *clist = new sgVec4[VRML_MAX_TEXTURES];
  current_material = new _ssgMaterial;
  sgSetVec4(current_material->spec,0.1f,0.1f,0.1f,1.0f);
  sgSetVec4(current_material->emis,0.0f,0.0f,0.0f,1.0f);
  sgSetVec4(current_material->rgb,0.3f,0.5f,0.1f,1.0f);
  current_material->shi = 0.0;
  
  
  sgSetVec2 ( texrep, 1.0, 1.0 ) ;
  loader_fd = fopen ( filename, "ra" ) ;

  //VRMLlog       = fopen ( "f:/VRMLlog", "w" );
  if ( loader_fd == NULL )
  {
    perror ( filename ) ;
    fprintf ( stderr, "ssgLoadVRML: Failed to open '%s' for reading\n", filename ) ;
    return NULL ;
  }
  
  char buffer [ 1024 ] ;
  int firsttime = TRUE ;
  
  current_branch = new ssgTransform () ;
  
  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
  {
    char *s = buffer ;
    /* VRMLskip leading whitespace */
    VRMLskip_spaces ( & s ) ;
    if ( firsttime )
    {
      firsttime = FALSE ;
      if ( strncasecmp ( s, "#VRML", 5 ) != 0 )
      {
        //fclose ( loader_fd ) ;
        fprintf ( stderr, "ssgLoadVRML: '%s' is not in VRML format.\n", filename ) ;
        //return NULL ;
      }
    }
    /* VRMLskip blank lines and comments */
    if ( *s < ' ' && *s != '\t' ) continue ;
    if ( *s == '#' || *s == ';' ) continue ;
    VRMLsearch ( VRMLtop_tags, s ) ;
  }
  
  int i ;

  for ( i = 0 ; i < num_textures ; i++ )
    delete texture_fnames [ i ] ;
  
  delete [] vtab ;
  fclose ( loader_fd ) ;
  
  //fprintf(VRMLlog,"\nVLISTS: %d",VLISTcnt);
  //for (i =0;i<VLISTcnt;i++){
    //fprintf(VRMLlog,"\nVLIST%d count: %d",i,VLISTverts[i]);
  //}
  //fprintf(VRMLlog,"\nILIST %d",ILISTcnt);
  
  //int nn = 1;
  //int nt = 1;
  //int nc = 1;
  
  //sgVec3 *n = new sgVec3[1];
  //sgVec2 *t = new sgVec2[1];
  sgVec4 *c = new sgVec4[1];
  
  iList   *iListCurrent;
  vecList *vListCurrent;
  nrmList *nListCurrent;
  niList  *niListCurrent;
  tiList  *tiListCurrent;
  texList  *tListCurrent;

  iList   *iListOld;
  vecList *vListOld;
  nrmList *nListOld;
  niList  *niListOld;
  tiList  *tiListOld;
  texList  *tListOld;

  if (current_colour[0] != NULL) {
    sgCopyVec4(c[0],current_colour[0]);
  } else { sgSetVec4(c[0],0.0f,0.3f,0.5f,1.0f); }
  
  ssgVTable *table;
  sgCoord *tmpCoord;
  tmpCoord = new sgCoord;
  
  ssgState *st = NULL;
  
  
  for (i = 0 ; i < VLISTcnt; i++){
    unsigned short *il = new unsigned short[ILISTn[i]];
    sgVec3 *vl = new sgVec3[ILISTn[i]];
    sgVec3 *tri = new sgVec3[ILISTn[i]];
    sgVec3 *n_tmp = new sgVec3[NLISTnrms[i]];
    sgVec3 *n     = new sgVec3[NILISTn[i]];
    sgVec2 *t    = new sgVec2[TILISTn[i]];
    sgVec2 *t_tmp = new sgVec2[TLISTverts[i]];

    //fprintf(VRMLlog,"\nsgVec3 array init'd to %d",ILISTn[i]);
    iListCurrent = ILIST[i];
    vListCurrent = VLIST[i];
    nListCurrent = NLIST[i];
    niListCurrent= NILIST[i];
    tiListCurrent= TILIST[i];
    tListCurrent = TLIST[i];

    //  fprintf(VRMLlog,"testing VLIST[0]: %f %f %f",vListCurrent->pnt[0],vListCurrent->pnt[1],vListCurrent->pnt[2]);

    int o ;

    for(o = 0 ; o < ILISTn[i]; o++){
      il[o]=iListCurrent->index;
      iListOld = iListCurrent;
      iListCurrent=iListCurrent->next;
      delete iListOld;
    }
    // first i make an array out of the list structure to allow indexing to work
    //fprintf(VRMLlog,"\nTEX TLISTverts[i] = %d",TLISTverts[i]);
    for(o=0;o<TLISTverts[i];o++){
      sgCopyVec2(t_tmp[o],tListCurrent->pnt);
      tListOld = tListCurrent;
      tListCurrent = tListCurrent->next;
      delete tListOld;
    }
    //fprintf(VRMLlog,"\nTEX TILISTn[i] = %d",TILISTn[i]);
    for(o=0;o<TILISTn[i];o++){
      //fprintf(VRMLlog,"\n%d tiListCurrent->index=%d ",o,tiListCurrent->index);
      //fprintf(VRMLlog,"\n u=%f v=%f",tListCurrent->pnt[0],tListCurrent->pnt[1]);
      //fprintf(VRMLlog,"\nTLIST[tiListCurrent->index]->pnt = %f %f",TLIST[tiListCurrent->index]->pnt[0],TLIST[tiListCurrent->index]->pnt[1]);
      sgCopyVec2(t[o],t_tmp[tiListCurrent->index]);
      //fprintf(VRMLlog,"\n o->%d  index->%d u:%f v:%f",o,tiListCurrent->index,t[o][0],t[o][1]);
      //t[o]=tiListCurrent->index;
      tiListOld = tiListCurrent;
      tiListCurrent=tiListCurrent->next;
      //delete tiListOld;
    }

    for(/*int*/ o = 0 ; o < VLISTverts[i]; o++){
      sgCopyVec3(vl[o],vListCurrent->pnt);
      //vl vListCurrent->pnt[0];
      //fprintf(VRMLlog,"vl[0][0] = %f %f %f",vl[o][0],vl[o][1],vl[o][2]);
      vListOld = vListCurrent;
      vListCurrent = vListCurrent->next;
      delete vListOld;
    }
    
    //read indices to put tri in correct order
    for(o = 0; o< ILISTn[i]; o++){
      sgCopyVec3(tri[o],vl[il[o]]);
    }
    
    for (o=0;o<NLISTnrms[i];o++){
      sgCopyVec3(n_tmp[o],nListCurrent->nrm);
      nListOld = nListCurrent;
      nListCurrent = nListCurrent->next;
      delete nListOld;
    }
    for (o=0;o<NILISTn[i];o++){
      sgCopyVec3(n[o],n_tmp[niListCurrent->index]);
      niListOld = niListCurrent;
      niListCurrent = niListCurrent->next;
      delete niListOld;
    }
    //generates incorrect normals
    /*
    for (o = 0 ; o < ILISTn[i] ; o++){
    sgMakeNormal ( n [ o ], tri[o],tri[o+1],tri[o+2]) ;
    o++; o++;
    }
    */
    
    //unsigned short nu = 1;
    // indexed version 
    /*
    table = new ssgVTable ( GL_TRIANGLES,
    VLISTverts[i], il, vl,
    0, &nu, NULL,
    0, &nu, NULL,
    1, &nu, &cl );
    */
    // triangles
    table = new ssgVTable ( GL_TRIANGLES,
            ILISTn[i], tri,
            NILISTn[i],  n,
            TILISTn[i],  t,
            1,  &colors[i] );
    table->setState(st);
    table->setCullFace(1);
    table->setName("VRML Table");
    ssgSimpleState *st = new ssgSimpleState () ;
    
    st -> setMaterial ( GL_SPECULAR, current_material -> spec ) ;
    st -> setMaterial ( GL_EMISSION, current_material -> emis ) ;
    st -> setShininess ( current_material -> shi ) ;
    //st -> setDiffuse( current_color[i]);
    st -> enable ( GL_COLOR_MATERIAL ) ;
    st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
    st -> enable  ( GL_LIGHTING       ) ;
    st -> setShadeModel ( GL_SMOOTH ) ;
    table->setState(st);
    
    if (texCnt > 0 && i < texCnt) {
      st->setTexture(tex_list[i]);
      st->enable(GL_TEXTURE_2D);
      //fprintf(VRMLlog,"\nAssigned TEXTURE\n  %d",i);

    }
    
    //((ssgTransform*)current_branch)->setTransform(xform[i]);
    //sgSetCoord(tmpCoord,xform[i],rot[i]);
    ssgTransform *tr = new ssgTransform;
    tr->setTransform(xform[i]);
    tr->addKid(table);
    current_branch->addKid(tr);
    delete [] vl;
    delete [] il;
    //delete [] tl;
    
    //delete table;
    //delete tri;
  }
  //delete table;
  current_branch->setName("current_branch");
  //fclose(VRMLlog);
  //for (i=0;i<VLISTcnt;i++)
  //{
  //}
  //change Zup to Yup
  ssgTransform *Zup = new ssgTransform;
  sgSetCoord(tmpCoord,0.0f,0.0f,0.0f,
    0.0f,90.0f,0.0f);
  Zup->setTransform(tmpCoord);
  Zup->addKid(current_branch);
  return Zup ;
}

