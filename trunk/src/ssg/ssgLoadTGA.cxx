
#include "ssgLocal.h"

#ifdef SSG_LOAD_TGA_SUPPORTED

#include <sys/stat.h>

/*
 * Submitted by Sam Stickland : sam@spacething.org
 * Targe loading code based on code written Dave Gay : f00Dave@bigfoot.com, http://personal.nbnet.nb.ca/daveg/
 */
bool ssgLoadTGA ( const char *fname, ssgTextureInfo* info )
{
#define DEF_targaHeaderLength  12
#define DEF_targaHeaderContent "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  
  struct stat fileinfo;
  int bytesRead, width, height, maxLen;
  char *pData = NULL;
  
  if ( stat(fname, &fileinfo) == -1 ) {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname);
    return false ;
  }
  
  FILE *tfile;
  if( (tfile = fopen(fname, "rb")) == NULL) {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname);
    return false ;
  }
  
  maxLen = fileinfo.st_size;
  pData  = (char *) malloc(maxLen);
  fread (pData, maxLen, 1, tfile);
  fclose (tfile);
  pData[0] = 0x00;
  
  if( memcmp( pData, DEF_targaHeaderContent, DEF_targaHeaderLength ) != 0 ) {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'. Not a targa (apparently).", fname);
    free (pData);
    return false ;
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
    free (pData);
    return false ;
  }
  
  // Only allow 24-bit and 32-bit!
  bool is24Bit( depth == 24 );
  bool is32Bit( depth == 32 );
  
  if( !( is24Bit || is32Bit ) )
  {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'. Not 24 or 32 bit.", fname);
    free (pData);
    return false ;
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
  
  if ( info != NULL )
  {
    info -> width = width ;
    info -> height = height ;
    info -> depth = 4 ;
    info -> alpha = is32Bit? 1: 0 ;
  }
  return ssgMakeMipMaps ( texels, width, height, 4) ;
}

#else

bool ssgLoadTGA ( const char *fname, ssgTextureInfo* info )
{
  ulSetError ( UL_WARNING,
    "ssgLoadTexture: '%s' - TGA support not configured", fname ) ;
  return false ;
}

#endif
