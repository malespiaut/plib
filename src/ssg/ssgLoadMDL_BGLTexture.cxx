/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "ssgLocal.h"
#include "ssgMSFSPalette.h"

#ifdef SSG_LOAD_MDL_BGL_TEXTURE_SUPPORTED
// This really simple (raw paletted) format is used by older MSFS for textures
bool ssgLoadMDLTexture ( const char *fname, ssgTextureInfo* info )
{
  FILE *tfile;
  int index = 0;
  if ( (tfile = fopen(fname, "rb")) == NULL) {
    char *p = strrchr(fname,'_');
    if (p != 0) {
      *p = '\0';
      p++;
      index = atoi (p);
      if ( (tfile = fopen(fname, "rb")) == NULL) {
        ulSetError( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname );
        return false ;
      }
      p--;
      *p = '_';
    }
    else {
      ulSetError( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname );
      return false ;
    }
  }

  fseek(tfile, 0, SEEK_END);
  unsigned long file_length = ftell(tfile);

  if (file_length != 65536) {
    // this is not a MSFS-formatted texture, so it's probably a BMP
    fclose(tfile);
    return ssgLoadBMP( fname, info );
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
        texels[c++] = (b<index)?0:255;
      }
    }
    
    fclose(tfile);

    if ( info != NULL )
    {
      info -> width = 256 ;
      info -> height = 256 ;
      info -> depth = 4 ;
      info -> alpha = TRUE ;  //??
    }

    return ssgMakeMipMaps ( texels, 256, 256, 4 ) ;
  }
}
#else

bool ssgLoadMDLTexture ( const char *fname, ssgTextureInfo* info )
{
  ulSetError ( UL_WARNING,
    "ssgLoadTexture: '%s' - MDL/BGL Texture support not configured", fname ) ;
  return false ;
}

#endif
