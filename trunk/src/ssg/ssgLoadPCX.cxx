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

//===========================================================================
//
// File: ssgLoadPCX.cxx
//
// Created: 12.12.2003
//
// Author: Wolfram Kuss
//
// Revision: $Id$
//
// Description: Loads PCX texture files. Only 8 bit indexed for now.
//
//
//===========================================================================

#include "ssgLocal.h"
typedef	unsigned char	UByte,*UByteP;
#include "pcx.h"


static char Palette[3*256];

#ifdef SSG_LOAD_PCX_SUPPORTED

int ReadFileIntoBuffer(const char *fname, UByte *&buffer, UByte *&bufferorig, unsigned long &file_length)
// Opens the file, allocates buffer of correct size to hold the file, reads it, closes it
{
	// **** open file ****
  FILE *tfile;
  if ( (tfile = fopen(fname, "rb")) == NULL) {
    ulSetError( UL_WARNING, "ssgLoadTexture: Failed to open file '%s' for reading.", fname );
    return false ;
  }

	// **** allocate buffer, read file into it and close file ****
	fseek(tfile, 0, SEEK_END);
  file_length = ftell(tfile);
	fseek(tfile, 0, SEEK_SET);

	buffer = new UByte[file_length];
	bufferorig = buffer;
	
  fread(buffer, file_length, 1, tfile);
  fclose(tfile);
	return true;
}

bool ssgLoadPCX ( const char *fname, ssgTextureInfo* info )
{
	UByte *buffer, *bufferorig;
	unsigned long file_length;
	if(!ReadFileIntoBuffer(fname, buffer, bufferorig, file_length))
		return false ;
	// **** "read" header and "analyse" it ****
	pcxHeaderType *ppcxHeader = (pcxHeaderType *) buffer;
	buffer += sizeof(pcxHeaderType);

	short width = ppcxHeader->xmax-ppcxHeader->x+1;
	short height = ppcxHeader->ymax-ppcxHeader->y+1;
	//p->isMasked = ((p->resOfRowanTexture & 0x200) != 0) ? 1 : 0;
	
  if ( info != NULL )
  {
    info -> width = width ;
    info -> height = height ;
    info -> depth = 4 ;
    info -> alpha = TRUE ;  //I think we have to set it to true always, since we always generate 4 bytes per pixel
  }

	// **** read body´; Do error checking ****
	long size = ((long)width)*height;
	UByte *pAlfa = NULL, * pBody = new UByte [size]; // 1 byte per texel
	UByte * pBodyorig = pBody;

	if(!ReadPCXBody(buffer, ppcxHeader, pBody))
// writes to pBody, which must have been allocated
	{
		delete [] buffer;
		delete [] pBody;
		ulSetError ( UL_WARNING,
			"ssgLoadTexture: '%s' - unsupported or broken PCX texture file", fname ) ;
		return false ;
	}
	if(*buffer++ != 12)
	{	ulSetError ( UL_WARNING, "ssgLoadTexture: '%s' - PCX files needs a '12' byte", fname ) ;
		return false ;
	}
	assert(bufferorig + file_length - 768 == buffer);
	// starting at "buffer", you can find the palette, 256 entrys with 3 bytes each
	// only true for version 5 (and possible later versions)=

// start alfa handling
	// PCX does not allow alfa, so to enable alfa, you need two files :-(, 
	// one for the body, for example abc.pcx and one for the alfa component, for example abc_trans.pcx
	if(fname[strlen(fname)-4]=='.')
	{	
		char *t, *s = new char[strlen(fname)+15];
		strcpy(s, fname);
		t=&(s[strlen(s)-4]);
		strcpy(t, "_trans.pcx");
		if(ulFileExists(s))
		{
			UByte *alfaBuffer, *alfaBufferorig;
			if(!ReadFileIntoBuffer(s, alfaBuffer, alfaBufferorig, file_length))
				return false ;
			// **** "read" header and "analyse" it ****
			ppcxHeader = (pcxHeaderType *) alfaBuffer;
			alfaBuffer += sizeof(pcxHeaderType);

			if(width != ppcxHeader->xmax-ppcxHeader->x+1)
				ulSetError ( UL_WARNING, "ssgLoadTexture: '%s' - Width does not agree to 'body' width, so alfa is ignored", s ) ;				
			else
			{
				
				if (height != ppcxHeader->ymax-ppcxHeader->y+1)
					ulSetError ( UL_WARNING, "ssgLoadTexture: '%s' - Height does not agree to 'body' height, so alfa is ignored", s ) ;				
				else
				{				
					// **** read body´; Do error checking ****
					pAlfa = new UByte [size]; // 1 byte per texel
					
					if(!ReadPCXBody(alfaBuffer, ppcxHeader, pAlfa))
					// writes to pBody, which must have been allocated
					{
						delete [] buffer;
						delete [] pAlfa;
						ulSetError ( UL_WARNING,
							"ssgLoadTexture: '%s' - unsupported or broken PCX texture file", fname ) ;
						return false ;
					}
				}
			}
		}
	}
// end alfa handling
	
  UByte *texels = new UByte [size * 4]; // 4 bytes per texel
  int c = 0;
	int iRunningIndex = 0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
			UByte a = pAlfa?pAlfa[iRunningIndex]:255; 
      UByte b = pBody[iRunningIndex++];
      texels[c++] = buffer[b*3    ];
      texels[c++] = buffer[b*3 + 1];
      texels[c++] = buffer[b*3 + 2];
      texels[c++] = a; 
    }
  }
  
	delete [] pBodyorig;
	delete [] bufferorig;
	if (pAlfa)
		delete [] pAlfa;
  return ssgMakeMipMaps ( texels, width, height, 4 ) ;
}
#else

bool ssgLoadPCX ( const char *fname, ssgTextureInfo* info )
{
  ulSetError ( UL_WARNING,
    "ssgLoadTexture: '%s' - PCX texture support not configured", fname ) ;
  return false ;
}

#endif

