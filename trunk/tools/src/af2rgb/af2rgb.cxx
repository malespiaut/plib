// af2rgb.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <stdio.h>
#include <plib/ul.h>
#include <plib/ssgMSFSPalette.h>

// *.?af textures are always 256 x 256:
#define XSIZE 256
#define YSIZE 256
#define ZSIZE 4

static unsigned char texelsR [ XSIZE * YSIZE ];
static unsigned char texelsG [ XSIZE * YSIZE ];
static unsigned char texelsB [ XSIZE * YSIZE ];
static unsigned char texelsA [ XSIZE * YSIZE ];

static FILE *fd;

static size_t writeByte (unsigned char x )
{
  return fwrite ( & x, sizeof(unsigned char), 1, fd ) ;
}

static size_t writeShort (unsigned short x )
{
  return fwrite( & x, sizeof(unsigned short), 1, fd ) ;
}

static size_t writeInt (unsigned int x)
{
  return fwrite( & x, sizeof(unsigned int), 1, fd ) ;
}




#define SGI_IMG_MAGIC           0x01DA

static int writeTexelsIntoSGI ( const char *fname )
{
  
  fd = fopen ( fname, "wb" ) ;
	size_t NoWritten = 0;

  if ( fd == NULL )
  {
    ulSetError(UL_WARNING, "ssgLoadTexture: Failed to open '%s' for writing.", fname ) ;
    return false;
  }

  // ******* write the header *****************

  NoWritten += writeShort (SGI_IMG_MAGIC) ;

  NoWritten += writeByte  (0) ; // don't use RLE
  NoWritten += writeByte  (1) ; // bpp
  NoWritten += writeShort (3) ; // dimension

  NoWritten += writeShort ( XSIZE ) ;
  NoWritten += writeShort ( YSIZE ) ;
  NoWritten += writeShort ( ZSIZE ) ;
  NoWritten += writeInt   ( 0 ) ;  
  NoWritten += writeInt   ( 255 ) ;  
  NoWritten += writeInt   ( 0 ) ;  /* Dummy field */

  int i ;

  for ( i = 0 ; i < 80 ; i++ )
    NoWritten += writeByte ( 0 ) ;         /* Name field */

  NoWritten += writeInt ( 0 ) ; // colormap

  for ( i = 0 ; i < 404 ; i++ )
    NoWritten += writeByte ( 0 ) ;         /* Dummy field */
  
  // ************ write the body ****************  

	NoWritten += fwrite ( texelsR, XSIZE, YSIZE, fd ) ;
  NoWritten += fwrite ( texelsG, XSIZE, YSIZE, fd ) ;
  NoWritten += fwrite ( texelsB, XSIZE, YSIZE, fd ) ;
  NoWritten += fwrite ( texelsA, XSIZE, YSIZE, fd ) ;
  
	if(NoWritten!=91+404+4*YSIZE)
	{ ulSetError(UL_WARNING, "Only %ld records written instead of %d. Proceed at your own risc\n", 
	                  (long)NoWritten, 91+404+4*YSIZE);

		return false;
	}
  fclose ( fd ) ;
	return true;
}



int loadMDLIntoTexels ( const char *fname )
// returns TRUE on success
{
  FILE *tfile;
  if ( (tfile = fopen(fname, "rb")) == NULL) {
    ulSetError(UL_WARNING,  "ssgLoadTexture: Failed to load '%s'.", fname );
    return false;
  }

  fseek(tfile, 0, SEEK_END);
  unsigned long file_length = ftell(tfile);

  if (file_length != 65536) {
    ulSetError(UL_WARNING, "ssgLoadTexture: Wrong size of '%s'.", fname );
    return false;
  } else {
    fseek(tfile, 0, SEEK_SET);

    int c = 0;
    for (int y = 0; y < 256; y++) {
      for (int x = 0; x < 256; x++) {
				unsigned char b;
				if ( fread(&b, 1, 1, tfile) != 1)
					return false;
				texelsR[c] = fsTexPalette[b*4    ];
				texelsG[c] = fsTexPalette[b*4 + 1];
				texelsB[c] = fsTexPalette[b*4 + 2];
				texelsA[c] = fsTexPalette[b*4 + 3];
				c++;
      }
    }
    fclose(tfile);
  }
	return true;
}

void DoAllFiles( char *sDirectoryP )
{ char newname [ 1024 ], sDirectory[1024], sFullPath[1024]; // todo fixme

	strcpy( sDirectory, sDirectoryP);
	if ( ( sDirectoryP[ strlen(sDirectoryP)-1 ] != '\\') &&
		   ( sDirectoryP[ strlen(sDirectoryP)-1 ] != '/') )
	  strcat( sDirectory, "/");
// ************** For all *.?af files in the current dir ************	
	ulDir* dirp = ulOpenDir(".");
  if ( dirp != NULL )
  {
    ulDirEnt* dp;
    while ( (dp = ulReadDir(dirp)) != NULL )
    {
      if ( !dp->d_isdir )
			{ size_t len = strlen(dp->d_name );
				if ( len >4 )
					if ((dp->d_name[len-1] == 'f') || (dp->d_name[len-1] == 'F'))
						if ((dp->d_name[len-2] == 'a') || (dp->d_name[len-2] == 'A'))
							if ((dp->d_name[len-4] == '.') || (dp->d_name[len-4] == '.'))
							{ strcpy( newname, dp->d_name );
								int i;
								for (i=0; i<len;  i++)
									newname [i] = tolower ( newname [i] );

								newname[len-4] = newname[len-3];
								newname[len-3] = '.';
								newname[len-2] = 'r';
								newname[len-1] = 'g';
								newname[len] = 'b';
								newname[len+1] = 0;
								ulSetError(UL_DEBUG, "%s   %s\n",dp->d_name, newname);
								// ********** convert it **************
								if ( loadMDLIntoTexels ( dp->d_name ))
								{	strcpy(sFullPath, sDirectory);
								  strcat(sFullPath, newname);
									writeTexelsIntoSGI ( sFullPath ); //lint !e534
								}
							}
			}
    }
    ulCloseDir(dirp);
	}
}

// converts all *.?af files from the current dir to *.rgb in the dir givven in the argument
int main(int argc, char* argv[])
{
	if ( argc > 2 )
		ulSetError(UL_WARNING, "All arguments after the first are ignored!" );
	if ( argc >= 2 )
    DoAllFiles( argv[1] );
	else
		DoAllFiles(".");
	return 0;
}

