/*******************************************************
 **  ssg3ds.h
 **  
 *   Common data for ssgLoad3ds.cxx and ssgSave3ds.cxx
 *******************************************************/

// 3ds chunk identifiers
static const int CHUNK_VERSION         = 0x0002;
static const int CHUNK_RGB1            = 0x0010;  // 3 floats of RGB
static const int CHUNK_RGB2            = 0x0011;  // 3 bytes of RGB
static const int CHUNK_RGB3            = 0x0012;  // 3 bytes of RGB (gamma)
static const int CHUNK_AMOUNT          = 0x0030;
static const int CHUNK_MAIN            = 0x4D4D;
static const int CHUNK_OBJMESH         = 0x3D3D;
static const int CHUNK_ONEUNIT         = 0x0100;
static const int CHUNK_BKGCOLOR        = 0x1200;
static const int CHUNK_AMBCOLOR        = 0x2100;
static const int CHUNK_OBJBLOCK        = 0x4000;
static const int CHUNK_TRIMESH         = 0x4100;
static const int CHUNK_VERTLIST        = 0x4110;
static const int CHUNK_FACELIST        = 0x4120;
static const int CHUNK_FACEMAT         = 0x4130;
static const int CHUNK_MAPLIST         = 0x4140;
static const int CHUNK_SMOOLIST        = 0x4150;
static const int CHUNK_TRMATRIX        = 0x4160;
static const int CHUNK_LIGHT           = 0x4600;
static const int CHUNK_SPOTLIGHT       = 0x4610;
static const int CHUNK_CAMERA          = 0x4700;
static const int CHUNK_MATERIAL        = 0xAFFF;
static const int CHUNK_MATNAME         = 0xA000;
static const int CHUNK_AMBIENT         = 0xA010;
static const int CHUNK_DIFFUSE         = 0xA020;
static const int CHUNK_SPECULAR        = 0xA030;
static const int CHUNK_SHININESS       = 0xA040;
static const int CHUNK_SHINE_STRENGTH  = 0xA041;
static const int CHUNK_TRANSPARENCY    = 0xA050;
static const int CHUNK_TRANSP_FALLOFF  = 0xA052;
static const int CHUNK_DOUBLESIDED     = 0xA081;
static const int CHUNK_TEXTURE         = 0xA200;
static const int CHUNK_BUMPMAP         = 0xA230;
static const int CHUNK_MAPFILENAME     = 0xA300;
static const int CHUNK_MAPOPTIONS      = 0xA351;
static const int CHUNK_MAP_VSCALE      = 0xA354;
static const int CHUNK_MAP_USCALE      = 0xA356;
static const int CHUNK_MAP_UOFFST      = 0xA358;
static const int CHUNK_MAP_VOFFST      = 0xA35A;
static const int CHUNK_KEYFRAMER       = 0xB000;
static const int CHUNK_FRAMES          = 0xB008;
