
#include "ssgLocal.h"
#include "ssgMSFSPalette.h"

#ifdef _SSG_USE_GLPNG
#include "glpng.h"
#endif

#include <sys/stat.h>

static FILE          *curr_image_fd ;
static char           curr_image_fname [ 512 ] ;
static int            isSwapped ;
static unsigned char *rle_temp ;
static int total_texels_loaded = 0 ;


static void loadTextureDummy () ;


void make_mip_maps ( GLubyte *image, int xsize, int ysize, int zsize )
{
  if ( ! ((xsize & (xsize-1))==0) ||
       ! ((ysize & (ysize-1))==0) )
  {
    ulSetError ( UL_WARNING, "%s: Map is not a power-of-two in size!", curr_image_fname ) ;
    loadTextureDummy () ;
    return ;
  }

  GLubyte *texels [ 20 ] ;   /* One element per level of MIPmap */

  for ( int l = 0 ; l < 20 ; l++ )
    texels [ l ] = NULL ;

  texels [ 0 ] = image ;

  int lev ;

  for ( lev = 0 ; (( xsize >> (lev+1) ) != 0 ||
                   ( ysize >> (lev+1) ) != 0 ) ; lev++ )
  {
    /* Suffix '1' is the higher level map, suffix '2' is the lower level. */

    int l1 = lev   ;
    int l2 = lev+1 ;
    int w1 = xsize >> l1 ;
    int h1 = ysize >> l1 ;
    int w2 = xsize >> l2 ;
    int h2 = ysize >> l2 ;

    if ( w1 <= 0 ) w1 = 1 ;
    if ( h1 <= 0 ) h1 = 1 ;
    if ( w2 <= 0 ) w2 = 1 ;
    if ( h2 <= 0 ) h2 = 1 ;

    texels [ l2 ] = new GLubyte [ w2 * h2 * zsize ] ;

    for ( int x2 = 0 ; x2 < w2 ; x2++ )
      for ( int y2 = 0 ; y2 < h2 ; y2++ )
        for ( int c = 0 ; c < zsize ; c++ )
        {
          int x1   = x2 + x2 ;
          int x1_1 = ( x1 + 1 ) % w1 ;
          int y1   = y2 + y2 ;
          int y1_1 = ( y1 + 1 ) % h1 ;

	  int t1 = texels [ l1 ] [ (y1   * w1 + x1  ) * zsize + c ] ;
	  int t2 = texels [ l1 ] [ (y1_1 * w1 + x1  ) * zsize + c ] ;
	  int t3 = texels [ l1 ] [ (y1   * w1 + x1_1) * zsize + c ] ;
	  int t4 = texels [ l1 ] [ (y1_1 * w1 + x1_1) * zsize + c ] ;

          texels [ l2 ] [ (y2 * w2 + x2) * zsize + c ] =
                                           ( t1 + t2 + t3 + t4 ) / 4 ;
        }
  }

  texels [ lev+1 ] = NULL ;

  glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 ) ;

  int map_level = 0 ;

#ifdef PROXY_TEXTURES_ARE_NOT_BROKEN
  int ww ;

  do
  {
    glTexImage2D  ( GL_PROXY_TEXTURE_2D,
                     map_level, zsize, xsize, ysize, FALSE /* Border */,
                            (zsize==1)?GL_LUMINANCE:
                            (zsize==2)?GL_LUMINANCE_ALPHA:
                            (zsize==3)?GL_RGB:
                                       GL_RGBA,
                            GL_UNSIGNED_BYTE, NULL ) ;

    glGetTexLevelParameteriv ( GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_WIDTH, &ww ) ;

    if ( ww == 0 )
    {
      delete texels [ 0 ] ;
      xsize >>= 1 ;
      ysize >>= 1 ;

      for ( int l = 0 ; texels [ l ] != NULL ; l++ )
	texels [ l ] = texels [ l+1 ] ;

      if ( xsize < 64 && ysize < 64 )
      {
        ulSetError ( UL_FATAL,
           "SSG: OpenGL will not accept a downsized version of '%s' ?!?",
           curr_image_fname ) ;
      }
    }
  } while ( ww == 0 ) ;
#endif

  for ( int i = 0 ; texels [ i ] != NULL ; i++ )
  {
    int w = xsize>>i ;
    int h = ysize>>i ;

    if ( w <= 0 ) w = 1 ;
    if ( h <= 0 ) h = 1 ;

    total_texels_loaded += w * h ;

    glTexImage2D  ( GL_TEXTURE_2D,
                     map_level, zsize, w, h, FALSE /* Border */,
                            (zsize==1)?GL_LUMINANCE:
                            (zsize==2)?GL_LUMINANCE_ALPHA:
                            (zsize==3)?GL_RGB:
                                       GL_RGBA,
                            GL_UNSIGNED_BYTE, (GLvoid *) texels[i] ) ;
    map_level++ ;
    delete texels [ i ] ;
  }
}


int ssgGetNumTexelsLoaded ()
{
  return total_texels_loaded ;
}


static void loadTextureDummy ()
{
  GLubyte *image = new GLubyte [ 4 * 3 ] ;

  /* Red and white chequerboard */

  image [ 0 ] = 255 ; image [ 1 ] =  0  ; image [ 2 ] =  0  ;
  image [ 3 ] = 255 ; image [ 4 ] = 255 ; image [ 5 ] = 255 ;
  image [ 6 ] = 255 ; image [ 7 ] = 255 ; image [ 8 ] = 255 ;
  image [ 9 ] = 255 ; image [ 10] =  0  ; image [ 11] =  0  ;

  make_mip_maps ( image, 2, 2, 3 ) ;
}


/* Some magic constants in the file header. */

#define SGI_IMG_MAGIC           0x01DA
#define SGI_IMG_SWABBED_MAGIC   0xDA01   /* This is how it appears on a PC */
#define SGI_IMG_VERBATIM        0
#define SGI_IMG_RLE             1

class ssgSGIHeader
{
public:    /* Yuk!  Need to hide some of this public stuff! */
  unsigned short magic ;
  int            max ;
  int            min ;
  int            colormap ;
  char           type ;
  char           bpp ;
  unsigned int  *start ;
  int           *leng ;
  unsigned short dim ;
  unsigned short xsize ;
  unsigned short ysize ;
  unsigned short zsize ;
  int           tablen ;

  ssgSGIHeader () ;
  void makeConsistant () ;
  void getRow   ( unsigned char *buf, int y, int z ) ;
  void getPlane ( unsigned char *buf, int z ) ;
  void getImage ( unsigned char *buf ) ;
  void readHeader () ;
} ;


void ssgSGIHeader::makeConsistant ()
{
  /*
    Sanity checks - and a workaround for buggy RGB files generated by
    the MultiGen Paint program because it will sometimes get confused
    about the way to represent maps with more than one component.

    eg   Y > 1, Number of dimensions == 1
         Z > 1, Number of dimensions == 2
  */

  if ( ysize > 1 && dim < 2 ) dim = 2 ;
  if ( zsize > 1 && dim < 3 ) dim = 3 ;
  if ( dim < 1 ) ysize = 1 ;
  if ( dim < 2 ) zsize = 1 ;
  if ( dim > 3 ) dim   = 3 ;
  if ( zsize < 1 && ysize == 1 ) dim = 1 ;
  if ( zsize < 1 && ysize != 1 ) dim = 2 ;
  if ( zsize >= 1 ) dim = 3 ;

  /*
    A very few SGI image files have 2 bytes per component - this
    library cannot deal with those kinds of files. 
  */

  if ( bpp == 2 )
  {
    ulSetError ( UL_FATAL, "ssgLoadTexture: Can't work with SGI images with %d bpp", bpp ) ;
  }

  bpp = 1 ;
  min = 0 ;
  max = 255 ;
  magic = SGI_IMG_MAGIC ;
  colormap = 0 ;
}


static void swab_short ( unsigned short *x )
{
  if ( isSwapped )
    *x = (( *x >>  8 ) & 0x00FF ) | 
         (( *x <<  8 ) & 0xFF00 ) ;
}

static void swab_int ( unsigned int *x )
{
  if ( isSwapped )
    *x = (( *x >> 24 ) & 0x000000FF ) | 
         (( *x >>  8 ) & 0x0000FF00 ) | 
         (( *x <<  8 ) & 0x00FF0000 ) | 
         (( *x << 24 ) & 0xFF000000 ) ;
}

static void swab_int_array ( int *x, int leng )
{
  if ( ! isSwapped )
    return ;

  for ( int i = 0 ; i < leng ; i++ )
  {
    *x = (( *x >> 24 ) & 0x000000FF ) | 
         (( *x >>  8 ) & 0x0000FF00 ) | 
         (( *x <<  8 ) & 0x00FF0000 ) | 
         (( *x << 24 ) & 0xFF000000 ) ;
    x++ ;
  }
}


static unsigned char readByte ()
{
  unsigned char x ;
  fread ( & x, sizeof(unsigned char), 1, curr_image_fd ) ;
  return x ;
}

static unsigned short readShort ()
{
  unsigned short x ;
  fread ( & x, sizeof(unsigned short), 1, curr_image_fd ) ;
  swab_short ( & x ) ;
  return x ;
}

static unsigned int readInt ()
{
  unsigned int x ;
  fread ( & x, sizeof(unsigned int), 1, curr_image_fd ) ;
  swab_int ( & x ) ;
  return x ;
}


void ssgSGIHeader::getRow ( unsigned char *buf, int y, int z )
{
  if ( y >= ysize ) y = ysize - 1 ;
  if ( z >= zsize ) z = zsize - 1 ;

  fseek ( curr_image_fd, start [ z * ysize + y ], SEEK_SET ) ;

  if ( type == SGI_IMG_RLE )
  {
    unsigned char *tmpp = rle_temp ;
    unsigned char *bufp = buf ;

    fread ( rle_temp, 1, leng [ z * ysize + y ], curr_image_fd ) ;

    unsigned char pixel, count ;

    while ( TRUE )
    {
      pixel = *tmpp++ ;

      count = ( pixel & 0x7f ) ;

      if ( count == 0 )
	break ;

      if ( pixel & 0x80 )
      {
        while ( count-- )
	  *bufp++ = *tmpp++ ;
      }
      else
      {
        pixel = *tmpp++ ;

	while ( count-- )
          *bufp++ = pixel ;
      }
    }
  }
  else
    fread ( buf, 1, xsize, curr_image_fd ) ;
}


void ssgSGIHeader::getPlane ( unsigned char *buf, int z )
{
  if ( curr_image_fd == NULL )
    return ;

  if ( z >= zsize ) z = zsize - 1 ;

  for ( int y = 0 ; y < ysize ; y++ )
    getRow ( & buf [ y * xsize ], y, z ) ;
}



void ssgSGIHeader::getImage ( unsigned char *buf )
{
  if ( curr_image_fd == NULL )
    return ;

  for ( int y = 0 ; y < ysize ; y++ )
    for ( int z = 0 ; z < zsize ; z++ )
      getRow ( & buf [ ( z * ysize + y ) * xsize ], y, z ) ;
}


ssgSGIHeader::ssgSGIHeader ()
{
  dim   = 0 ;
  start = NULL ;
  leng  = NULL ;
  rle_temp = NULL ;
}


void ssgSGIHeader::readHeader ()
{
  isSwapped = FALSE ;

  magic = readShort () ;

  if ( magic != SGI_IMG_MAGIC && magic != SGI_IMG_SWABBED_MAGIC )
  {
    ulSetError ( UL_FATAL, "%s: Unrecognised magic number 0x%04x",
                                         curr_image_fname, magic ) ;
  }

  if ( magic == SGI_IMG_SWABBED_MAGIC )
  {
    isSwapped = TRUE ;
    swab_short ( & magic ) ;
  }

  type  = readByte  () ;
  bpp   = readByte  () ;
  dim   = readShort () ;

  /*
    This is a backstop test - if for some reason the magic number isn't swabbed, this
    test will still catch a swabbed file. Of course images with more than 256 dimensions
    are not catered for :-)
  */

  if ( dim > 255 )
  {
    ulSetError ( UL_WARNING, "%s: Bad swabbing?!?", curr_image_fname ) ;
    isSwapped = ! isSwapped ;
    swab_short ( & dim ) ;
    magic = SGI_IMG_MAGIC ;
  }

  xsize = readShort () ;
  ysize = readShort () ;
  zsize = readShort () ;
  min   = readInt   () ;  
  max   = readInt   () ;  
                 readInt   () ;  /* Dummy field */

  int i ;

  for ( i = 0 ; i < 80 ; i++ )
    readByte () ;         /* Name field */

  colormap = readInt () ;

  for ( i = 0 ; i < 404 ; i++ )
    readByte () ;         /* Dummy field */

  makeConsistant () ;

  tablen = ysize * zsize ;
  start = new unsigned int [ tablen ] ;
  leng  = new int [ tablen ] ;
}


static void loadTextureSGI ( const char *fname )
{
  ssgSGIHeader *sgihdr = new ssgSGIHeader () ;

  strcpy ( curr_image_fname, fname ) ;
  curr_image_fd = fopen ( curr_image_fname, "rb" ) ;

  if ( curr_image_fd == NULL )
  {
    perror ( "ssgLoadTexture" ) ;
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to open '%s' for reading.", curr_image_fname ) ;
    loadTextureDummy () ;
    return ;
  }

  sgihdr -> readHeader () ;

  if ( sgihdr -> type == SGI_IMG_RLE )
  {
    fread ( sgihdr->start, sizeof (unsigned int), sgihdr->tablen, curr_image_fd ) ;
    fread ( sgihdr->leng , sizeof (int), sgihdr->tablen, curr_image_fd ) ;
    swab_int_array ( (int *) sgihdr->start, sgihdr->tablen ) ;
    swab_int_array ( (int *) sgihdr->leng , sgihdr->tablen ) ;

    int maxlen = 0 ;

    for ( int i = 0 ; i < sgihdr->tablen ; i++ )
      if ( sgihdr->leng [ i ] > maxlen )
        maxlen = sgihdr->leng [ i ] ;

    rle_temp = new unsigned char [ maxlen ] ;
  }
  else
  {
    rle_temp = NULL ;

    for ( int i = 0 ; i < sgihdr->zsize ; i++ )
      for ( int j = 0 ; j < sgihdr->ysize ; j++ )
      {
        sgihdr->start [ i * sgihdr->ysize + j ] = sgihdr->xsize * ( i * sgihdr->ysize + j ) + 512 ;
        sgihdr->leng  [ i * sgihdr->ysize + j ] = sgihdr->xsize ;
      }
  }

  if ( sgihdr->zsize <= 0 || sgihdr->zsize > 4 )
  {
    ulSetError ( UL_FATAL, "ssgLoadTexture: '%s' is corrupted.", curr_image_fname ) ;
  }

  GLubyte *image = new GLubyte [ sgihdr->xsize *
                                 sgihdr->ysize *
                                 sgihdr->zsize ] ;

  GLubyte *ptr = image ;

  unsigned char *rbuf =                     new unsigned char [ sgihdr->xsize ] ;
  unsigned char *gbuf = (sgihdr->zsize>1) ? new unsigned char [ sgihdr->xsize ] : (unsigned char *) NULL ;
  unsigned char *bbuf = (sgihdr->zsize>2) ? new unsigned char [ sgihdr->xsize ] : (unsigned char *) NULL ;
  unsigned char *abuf = (sgihdr->zsize>3) ? new unsigned char [ sgihdr->xsize ] : (unsigned char *) NULL ;

  for ( int y = 0 ; y < sgihdr->ysize ; y++ )
  {
    int x ;

    switch ( sgihdr->zsize )
    {
      case 1 :
	sgihdr->getRow ( rbuf, y, 0 ) ;

	for ( x = 0 ; x < sgihdr->xsize ; x++ )
	  *ptr++ = rbuf [ x ] ;

	break ;

      case 2 :
	sgihdr->getRow ( rbuf, y, 0 ) ;
	sgihdr->getRow ( gbuf, y, 1 ) ;

	for ( x = 0 ; x < sgihdr->xsize ; x++ )
	{
	  *ptr++ = rbuf [ x ] ;
	  *ptr++ = gbuf [ x ] ;
	}
	break ;

      case 3 :
        sgihdr->getRow ( rbuf, y, 0 ) ;
	sgihdr->getRow ( gbuf, y, 1 ) ;
	sgihdr->getRow ( bbuf, y, 2 ) ;

	for ( x = 0 ; x < sgihdr->xsize ; x++ )
	{
	  *ptr++ = rbuf [ x ] ;
	  *ptr++ = gbuf [ x ] ;
	  *ptr++ = bbuf [ x ] ;
	}
	break ;

      case 4 :
        sgihdr->getRow ( rbuf, y, 0 ) ;
	sgihdr->getRow ( gbuf, y, 1 ) ;
	sgihdr->getRow ( bbuf, y, 2 ) ;
	sgihdr->getRow ( abuf, y, 3 ) ;

	for ( x = 0 ; x < sgihdr->xsize ; x++ )
	{
	  *ptr++ = rbuf [ x ] ;
	  *ptr++ = gbuf [ x ] ;
	  *ptr++ = bbuf [ x ] ;
	  *ptr++ = abuf [ x ] ;
	}
	break ;
    }
  }

  fclose ( curr_image_fd ) ;

  delete rbuf   ;
  delete gbuf   ;
  delete bbuf   ;
  delete abuf   ;

  _ssgAlphaFlag = ( sgihdr->zsize == 4 ) ;
  make_mip_maps ( image, sgihdr->xsize, sgihdr->ysize, sgihdr->zsize ) ;

  delete sgihdr ;
}


/*
  Original source for BMP loader kindly
  donated by "Sean L. Palmer" <spalmer@pobox.com>
*/


struct BMPHeader
{
  unsigned short  FileType      ;
  unsigned int    FileSize      ;
  unsigned short  Reserved1     ;
  unsigned short  Reserved2     ;
  unsigned int    OffBits       ;
  unsigned int    Size          ;
  unsigned int    Width         ;
  unsigned int    Height        ;
  unsigned short  Planes        ;
  unsigned short  BitCount      ;
  unsigned int    Compression   ;
  unsigned int    SizeImage     ;
  unsigned int    XPelsPerMeter ;
  unsigned int    YPelsPerMeter ;
  unsigned int    ClrUsed       ;
  unsigned int    ClrImportant  ;
} ;


struct RGBA
{
  unsigned char r,g,b,a ;
} ;


static void loadTextureBMP ( const char *fname )
{
  int w, h, bpp ;
  RGBA pal [ 256 ] ;

  BMPHeader bmphdr ;

  /* Open file & get size */

  strcpy ( curr_image_fname, fname ) ;

  if ( ( curr_image_fd = fopen ( curr_image_fname, "rb" ) ) == NULL )
  {
    perror ( "ssgLoadTexture" ) ;
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to open '%s' for reading.", curr_image_fname ) ;
    return ;
  }

  /*
    Load the BMP piecemeal to avoid struct packing issues
  */

  isSwapped = FALSE ;
  bmphdr.FileType = readShort () ;

  if ( bmphdr.FileType == ((int)'B' + ((int)'M'<<8)) )
    isSwapped = FALSE ;
  else
  if ( bmphdr.FileType == ((int)'M' + ((int)'B'<<8)) )
    isSwapped = TRUE  ;
  else
  {
    ulSetError ( UL_WARNING, "%s: Unrecognised magic number 0x%04x",
                            curr_image_fname, bmphdr.FileType ) ;
    return ;
  }

  bmphdr.FileSize      = readInt   () ;
  bmphdr.Reserved1     = readShort () ;
  bmphdr.Reserved2     = readShort () ;
  bmphdr.OffBits       = readInt   () ;
  bmphdr.Size          = readInt   () ;
  bmphdr.Width         = readInt   () ;
  bmphdr.Height        = readInt   () ;
  bmphdr.Planes        = readShort () ;
  bmphdr.BitCount      = readShort () ;
  bmphdr.Compression   = readInt   () ;
  bmphdr.SizeImage     = readInt   () ;
  bmphdr.XPelsPerMeter = readInt   () ;
  bmphdr.YPelsPerMeter = readInt   () ;
  bmphdr.ClrUsed       = readInt   () ;
  bmphdr.ClrImportant  = readInt   () ;
 
  w   = bmphdr.Width  ;
  h   = bmphdr.Height ;
  bpp = bmphdr.Planes * bmphdr.BitCount ;

#ifdef PRINT_BMP_HEADER_DEBUG
  ulSetError ( UL_DEBUG, "Filetype %04x",      bmphdr.FileType      ) ;
  ulSetError ( UL_DEBUG, "Filesize %08x",      bmphdr.FileSize      ) ;
  ulSetError ( UL_DEBUG, "R1 %04x",            bmphdr.Reserved1     ) ;
  ulSetError ( UL_DEBUG, "R2 %04x",            bmphdr.Reserved2     ) ;
  ulSetError ( UL_DEBUG, "Offbits %08x",       bmphdr.OffBits       ) ;
  ulSetError ( UL_DEBUG, "Size %08x",          bmphdr.Size          ) ;
  ulSetError ( UL_DEBUG, "Width %08x",         bmphdr.Width         ) ;
  ulSetError ( UL_DEBUG, "Height %08x",        bmphdr.Height        ) ;
  ulSetError ( UL_DEBUG, "Planes %04x",        bmphdr.Planes        ) ;
  ulSetError ( UL_DEBUG, "Bitcount %04x",      bmphdr.BitCount      ) ;
  ulSetError ( UL_DEBUG, "Compression %08x",   bmphdr.Compression   ) ;
  ulSetError ( UL_DEBUG, "SizeImage %08x",     bmphdr.SizeImage     ) ;
  ulSetError ( UL_DEBUG, "XPelsPerMeter %08x", bmphdr.XPelsPerMeter ) ;
  ulSetError ( UL_DEBUG, "YPelsPerMeter %08x", bmphdr.YPelsPerMeter ) ;
  ulSetError ( UL_DEBUG, "ClrUsed %08x",       bmphdr.ClrUsed       ) ;
  ulSetError ( UL_DEBUG, "ClrImportant %08x",  bmphdr.ClrImportant  ) ;
#endif

  int isMonochrome = TRUE ;
  int isOpaque     = TRUE ;

  if ( bpp <= 8 )
  {
    for ( int i = 0 ; i < 256 ; i++ )
    {
      pal[i].b = readByte () ;
      pal[i].g = readByte () ;
      pal[i].r = readByte () ;

      /* According to BMP specs, this fourth value is not really alpha value
	 but just a filler byte, so it is ignored for now. */
      pal[i].a = readByte () ;
      //if ( pal[i].a != 255 ) isOpaque = FALSE ;

      if ( pal[i].r != pal[i].g ||
           pal[i].g != pal[i].b ) isMonochrome = FALSE ;
    }
  }

  fseek ( curr_image_fd, bmphdr.OffBits, SEEK_SET ) ;

  bmphdr.SizeImage = w * h * (bpp / 8) ;
  GLubyte *data = new GLubyte [ bmphdr.SizeImage ] ;

  /* read and flip image */
  {
    int row_size = w * (bpp / 8) ;
    for ( int y = h-1 ; y >= 0 ; y-- )
    {
      GLubyte *row_ptr = &data [ y * row_size ] ;
      if ( fread ( row_ptr, 1, row_size, curr_image_fd ) != (unsigned)row_size )
      {
        ulSetError ( UL_WARNING, "Premature EOF in '%s'", curr_image_fname ) ;
        return ;
      }
    }
  }

  fclose ( curr_image_fd ) ;

  GLubyte *image ;
  int z ;

  if ( bpp == 8 )
  {
    if ( isMonochrome )
      z = isOpaque ? 1 : 2 ;
    else
      z = isOpaque ? 3 : 4 ;

    image = new GLubyte [ w * h * z ] ;

    for ( int i = 0 ; i < w * h ; i++ )
      switch ( z )
      {
        case 1 : image [ i       ] = pal[data[i]].r ; break ;
        case 2 : image [ i*2     ] = pal[data[i]].r ;
                 image [ i*2 + 1 ] = pal[data[i]].a ; break ;
        case 3 : image [ i*3     ] = pal[data[i]].r ;
                 image [ i*3 + 1 ] = pal[data[i]].g ;
                 image [ i*3 + 2 ] = pal[data[i]].b ; break ;
        case 4 : image [ i*4     ] = pal[data[i]].r ;
                 image [ i*4 + 1 ] = pal[data[i]].g ;
                 image [ i*4 + 2 ] = pal[data[i]].b ;
                 image [ i*4 + 3 ] = pal[data[i]].a ; break ;
        default : break ;
      }

    delete data ;
  }
  else
  if ( bpp == 24 )
  {
    z = 3 ;
    image = data ;

    /* BGR --> RGB */

    for ( int i = 0 ; i < w * h ; i++ )
    {
      GLubyte tmp = image [ 3 * i ] ;
      image [ 3 * i ] = image [ 3 * i + 2 ];
      image [ 3 * i + 2 ] = tmp ;
    }
  }
  else
  if ( bpp == 32 )
  {
    z = 4 ;
    image = data ;

    /* BGRA --> RGBA */

    for ( int i = 0 ; i < w * h ; i++ )
    {
      GLubyte tmp = image [ 4 * i ] ;
      image [ 4 * i ] = image [ 4 * i + 2 ];
      image [ 4 * i + 2 ] = tmp ;
    }
  }
  else
  {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Can't load %d bpp BMP textures.", bpp ) ;
    loadTextureDummy () ;
    return ;
  }

  _ssgAlphaFlag = ( z == 4 ) ;
  make_mip_maps ( image, w, h, z ) ;
}


void loadTexturePNG ( const char *fname )
{
#ifdef _SSG_USE_GLPNG
  pngInfo info;
  if (!pngLoad(fname, PNG_BUILDMIPMAP, PNG_ALPHA, &info)) {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname ) ;
    return ;
  }
  _ssgAlphaFlag = ( info.Alpha > 0 ) ;
#else
  ulSetError ( UL_WARNING, "ssgLoadTexture: '%s' - you need glpng for PNG format support",
        fname ) ;
  loadTextureDummy () ;
#endif
}


// This really simple (raw paletted) format is used by older MSFS for textures
void loadTextureMDL ( const char *fname )
{
  FILE *tfile;
  if ( (tfile = fopen(fname, "rb")) == NULL) {
    ulSetError( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname );
    loadTextureDummy();
    return;
  }

  fseek(tfile, 0, SEEK_END);
  unsigned long file_length = ftell(tfile);

  if (file_length != 65536) {
    // this is not a MSFS-formatted texture, so it's probably a BMP
    fclose(tfile);
    loadTextureBMP( fname );
    return;
  } else {
    fseek(tfile, 0, SEEK_SET);

    unsigned char *texels = new unsigned char[256 * 256 * 4];
    int c = 0;
    for (int y = 0; y < 256; y++) {
      for (int x = 0; x < 256; x++) {
	unsigned char b;
	fread(&b, 1, 1, tfile);
	texels[c++] = fsTexPalette[b*4    ];
	texels[c++] = fsTexPalette[b*4 + 1];
	texels[c++] = fsTexPalette[b*4 + 2];
	texels[c++] = fsTexPalette[b*4 + 3];
      }
    }

    fclose(tfile);
    
    // _ssgAlphaFlag = true ; ??
    make_mip_maps ( texels, 256, 256, 4 ) ;
  }
}

/*
 * Submitted by Sam Stickland : sam@spacething.org
 * Targe loading code based on code written Dave Gay : f00Dave@bigfoot.com, http://personal.nbnet.nb.ca/daveg/
 */
void loadTextureTGA ( const char *fname )
{
  strcpy ( curr_image_fname, fname ) ;

  #define DEF_targaHeaderLength  12
  #define DEF_targaHeaderContent "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"

  struct stat fileinfo;
  int bytesRead, width, height, maxLen;
  char *pData = NULL;

  if ( stat(fname, &fileinfo) == -1 ) {
	ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname);
	loadTextureDummy ();
	return;
  }

  FILE *tfile;
  if( (tfile = fopen(fname, "rb")) == NULL) {
	ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname);
	loadTextureDummy ();
	return;
  }

  maxLen = fileinfo.st_size;
  pData  = (char *) malloc(maxLen);
  fread (pData, maxLen, 1, tfile);
  fclose (tfile);
  pData[0] = 0x00;

  if( memcmp( pData, DEF_targaHeaderContent, DEF_targaHeaderLength ) != 0 ) {
	ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'. Not a targa (apparently).", fname);
	loadTextureDummy ();
	free (pData);
    return;
  }

  unsigned char smallArray[ 2 ];

  memcpy( smallArray, pData + DEF_targaHeaderLength + 0, 2 );
  width = smallArray[ 0 ] + smallArray[ 1 ] * 0x0100;

  memcpy( smallArray, pData + DEF_targaHeaderLength + 2, 2 );
  height = smallArray[ 0 ] + smallArray[ 1 ] * 0x0100;

  memcpy( smallArray, pData + DEF_targaHeaderLength + 4, 2 );
  int depth = smallArray[ 0 ];
  // + smallArray[ 1 ] * 0x0100;

  if( ( width <= 0 ) || ( height <= 0 ) )
  {
	ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'. Width and height < 0.", fname);
	loadTextureDummy ();
	free (pData);
    return;
  }

  // Only allow 24-bit and 32-bit!
  bool is24Bit( depth == 24 );
  bool is32Bit( depth == 32 );

  if( !( is24Bit || is32Bit ) )
  {
	ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'. Not 24 or 32 bit.", fname);
	loadTextureDummy ();
	free (pData);
    return;
  }

  // Make it a BGRA array for now.
  int bodySize( width * height * 4 );
  unsigned char * texels = new unsigned char[ bodySize ];
  if( is32Bit )
  {
	// Texture is 32 bit
    // Easy, just copy it.
    memcpy( texels, pData + DEF_targaHeaderLength + 6, bodySize );
  }
  else if( is24Bit )
  {
	// Texture is 24 bit
    bytesRead = DEF_targaHeaderLength + 6;
    for( int loop = 0; loop < bodySize; loop += 4, bytesRead += 3 )
    {
	  memcpy( texels + loop, pData + bytesRead, 3 );
      texels[ loop + 3 ] = 255;                      // Force alpha to max.
    }
  }

  // Swap R & B (convert to RGBA).
  for( int loop = 0; loop < bodySize; loop += 4 )
  {
    unsigned char tempC = texels[ loop + 0 ];
    texels[ loop + 0 ] = texels[ loop + 2 ];
    texels[ loop + 2 ] = tempC;
  }

  free(pData);

  _ssgAlphaFlag = is32Bit ;
  make_mip_maps ( texels, width, height, 4) ;
}

struct _ssgTextureFormat
{
  char *extension ;
  void (*loadfunc) ( const char * ) ;
} ;


static _ssgTextureFormat formats[] =
{
  { ".tga" ,   loadTextureTGA },
  { ".bmp" ,   loadTextureBMP },
  { ".png" ,   loadTexturePNG },
  { ".rgb" ,   loadTextureSGI },
  { ".rgba" ,  loadTextureSGI },
  { ".int" ,   loadTextureSGI },
  { ".inta" ,  loadTextureSGI },
  { ".bw" ,    loadTextureSGI },
  { ".0af" ,   loadTextureMDL },
  { ".1af" ,   loadTextureMDL },
  { ".2af" ,   loadTextureMDL },
  { ".3af" ,   loadTextureMDL },
  { ".4af" ,   loadTextureMDL },
  { ".5af" ,   loadTextureMDL },
  { ".6af" ,   loadTextureMDL },
  { ".7af" ,   loadTextureMDL },
  { ".8af" ,   loadTextureMDL },
  { ".9af" ,   loadTextureMDL },
  { ".aaf" ,   loadTextureMDL },
  { ".baf" ,   loadTextureMDL },
  { ".caf" ,   loadTextureMDL },
  { ".daf" ,   loadTextureMDL },
  { ".eaf" ,   loadTextureMDL },
  { ".faf" ,   loadTextureMDL },
  { ".gaf" ,   loadTextureMDL },
  { ".haf" ,   loadTextureMDL },
  { ".iaf" ,   loadTextureMDL },
  { ".jaf" ,   loadTextureMDL },
  { ".kaf" ,   loadTextureMDL },
  { NULL ,     NULL           }
} ;


void ssgLoadTexture ( const char *fname )
{
  _ssgAlphaFlag = false ;

  if ( fname == NULL || *fname == '\0' )
    return ;

  //find extension
  const char *extn = & ( fname [ strlen(fname) ] ) ;
  while ( extn != fname && *extn != '/' && *extn != '.' )
    extn-- ;

  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Cannot determine file type for '%s'", fname );
    return ;
  }

  for ( _ssgTextureFormat *f = formats; f->extension != NULL; f++ )
    if ( f->loadfunc != NULL &&
         _ssgStrNEqual ( extn, f->extension, strlen(f->extension) ) )
    {
      f->loadfunc( fname ) ;
      return ;
    }

  ulSetError ( UL_WARNING, "ssgLoadTexture: Unrecognised file type '%s'", extn ) ;
  loadTextureDummy () ;
}
