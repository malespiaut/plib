/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/

/*******************************************************
 **  ssgSaveFLT.cxx
 **  
 **  Written by Steve Baker
 **  Last updated: 2002-10-15
 **
 **  This was written to be a part of Stephen J Bakers
 **  PLIB (http://plib.sourceforge.net)
 *******************************************************/

#include <stdio.h>
#include "ssgLocal.h"

static FILE* save_fd;

static int writeChar ( char val )
{
  fwrite ( & val, sizeof(char), 1, save_fd ) ;
  return sizeof(char) ;
}


static int writeShort ( short val )
{
  writeChar ( (val >> 8) & 0xFF ) ;
  writeChar ( val & 0xFF ) ;
  return sizeof(short) ;
}


static int writeInt( int val )
{
  writeChar ( (val >> 24) & 0xFF ) ;
  writeChar ( (val >> 16) & 0xFF ) ;
  writeChar ( (val >> 8 ) & 0xFF ) ;
  writeChar ( val & 0xFF ) ;
  return sizeof(int) ;
}


void swabInt ( int *x )
{
  unsigned int t = (unsigned int) *x ;

  *x = ((t & 0x000000FF) << 24) |
       ((t & 0x0000FF00) <<  8) |
       ((t & 0x00FF0000) >>  8) |
       ((t & 0xFF000000) >> 24) ;
}


void swabDouble ( double *x )
{
  int *a = (int *) x ;
  int *b = a+1 ;

  swabInt ( a ) ;
  swabInt ( b ) ;

  *a = *a ^ *b ;
  *b = *a ^ *b ;
  *a = *a ^ *b ;
}



static int writeDouble( double val )
{
  if ( ulIsLittleEndian )
    swabDouble ( & val ) ;

  fwrite ( & val, sizeof(double), 1, save_fd ) ;
  return sizeof(double) ;
}


static int writeString( char *str, int slen )
{
  fwrite ( str, sizeof(char), slen, save_fd ) ;
  return slen ;
}



static void writeHeader ()
{
  int  len = 0;

  len += writeShort ( 1 ) ;
  len += writeShort ( 2+2+8+4+4+32+2+2+2+2+2+1+1+4+4+
                      4+4+4+4+4+4+4+4+4+4+4+4+4+2+2+
                      4+8+8+8+8+2+2+4+4+2+2+2+2+4+8+
                      8+8+8+8+8+8+8+2+2+2+2+2+2+2+2+4 ) ;
  len += writeString( "1234567", 8 ) ;
  len += writeInt   ( 1550 ) ;
  len += writeInt   ( 0 ) ;
  len += writeString( "1234567890123456789012345678901", 32 ) ;
  len += writeShort ( 3 ) ;
  len += writeShort ( 2 ) ;
  len += writeShort ( 2 ) ;
  len += writeShort ( 14 ) ;
  len += writeShort ( 1 ) ;
  len += writeChar  ( 0 ) ;  /* Meters */
  len += writeChar  ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;

  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;

  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;
  len += writeInt   ( 0 ) ;

  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;

  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;

  len += writeInt   ( 0 ) ;
  len += writeInt   ( 0 ) ;

  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;

  len += writeInt   ( 0 ) ;

  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeDouble( 0 ) ;
  len += writeShort ( 0 ) ;

  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;

  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;
  len += writeShort ( 0 ) ;
  len += writeInt   ( 0 ) ;

  writeShort ( len ) ;
}


void writePush () { writeShort ( 10 ) ; writeShort (  4 ) ; }
void writePop  () { writeShort ( 11 ) ; writeShort (  4 ) ; }

void writeObject ()
{
  writeShort ( 4 ) ;
  writeShort ( 2+2+8+4+2+2+2+2+2+2 ) ;
  writeString ( "1234567", 8 ) ;  /* Name */
  writeInt   ( 0x00000000 ) ;     /* Flags */
  writeShort ( 0 ) ;     /* Relative Priority */
  writeShort ( 0 ) ;     /* Transparency (1-Alpha) */
  writeShort ( 0 ) ;    /* Special ID 1 */
  writeShort ( 100 ) ;  /* Special ID 2 */
  writeShort ( 0 ) ;    /* Significance */
  writeShort ( 0 ) ;    /* Layer */
}



void writeFace ()
{
  writeShort ( 5 ) ;
  writeShort ( 2+2+8+4+2+1+1+2+2+1+1+2+2+2+2+2+4+2+1+1+
               4+1+1+2+4+4+4+2+2+4+4+2+2 ) ;
  writeString ( "1234567", 8 ) ;  /* Name */
  writeInt   ( 0 ) ;     /* IR color code */
  writeShort ( 0 ) ;     /* Relative Priority */
  writeChar  ( 0 ) ;     /* Draw Type */
  writeChar  ( 0 ) ;    /* Texture-White */
  writeShort ( 0 ) ;/* XXXX */ /* Color name index */
  writeShort ( 0 ) ;    /* Alternate Color name index */
  writeChar  ( 0 ) ;
  writeChar  ( 0 ) ;    /* Billboard */
  writeShort (-1 ) ;    /* Detail Texture */
  writeShort (-1 ) ;/* XXXX */ /* Texture */
  writeShort (-1 ) ;    /* Material */
  writeShort ( 0 ) ;/* XXXX */ /* SMC */
  writeShort ( 0 ) ;    /* DFAD FID */
  writeInt   ( 0 ) ;    /* IR Material Code */
  writeShort ( 0 ) ;    /* Transparenct */
  writeChar  ( 0 ) ;    /* LOD generation control */
  writeChar  ( 0 ) ;    /* Line Style */
  writeInt   ( 1<<3 ) ; /* Flags == Packed Colour */
  writeChar  ( 0 ) ;    /* Light mode */
  writeChar  ( 0 ) ;
  writeShort ( 0 ) ;
  writeInt   ( 0 ) ;
  writeInt   ( 0xFFFFFFFF ) ;  /* Packed Color */
  writeInt   ( 0xFFFFFFFF ) ;  /* Packed Alt Color */
  writeShort ( 0 ) ;/* XXXX */ /* Texture mapping index */
  writeShort ( 0 ) ;
  writeInt   ( 0 ) ;   /* Primary Color Index */
  writeInt   ( 0 ) ;   /* Secondary Color Index */
  writeShort ( 0 ) ;
  writeShort ( 0 ) ;
}

void writeLeaf ( ssgLeaf *leaf )
{
  writeObject () ;
  writePush   () ;
  writePop    () ;

  for ( int i = 0 ; i < leaf -> getNumTriangles () ; i++ )
  {
    writeFace () ;
    writePush () ;

    writePop  () ;
  }
}


void writeGroup ( ssgBranch *bra )
{
  writeShort ( 2 ) ;
  writeShort ( 2+2+8+2+2+4+2+2+2+1+1+4 ) ;
  writeString ( "1234567", 8 ) ;  /* Name */
  writeShort ( 0 ) ;              /* Relative Priority */
  writeShort ( 0 ) ;
  writeInt   ( 0x00000003 ) ;     /* Flags */
  writeShort ( 0 ) ;    /* Special ID 1 */
  writeShort ( 100 ) ;  /* Special ID 2 */
  writeShort ( 0 ) ;    /* Significance */
  writeChar  ( 0 ) ;    /* Layer */
  writeChar  ( 0 ) ;
  writeInt   ( 0 ) ;
}


void writeEntity ( ssgEntity *ent )
{
  if ( ent == NULL )
    return ;

  if ( ! ent -> isAKindOf ( ssgTypeBranch () ) )
  {
    writeLeaf ( (ssgLeaf *)ent ) ;
    return ;
  }  

  writeGroup ( (ssgBranch *)ent ) ;

  writePush () ;

  for ( int i = 0 ; i < ((ssgBranch *)ent) -> getNumKids() ; i++ )
    writeEntity ( ((ssgBranch *)ent) -> getKid ( i ) ) ;

  writePop () ;
  return ;
}


int ssgSaveFLT ( const char *fname, ssgEntity *root )
{
  save_fd = NULL ;

  if ( ( save_fd = fopen ( fname, "wb") ) == NULL )
  {
    fprintf ( stderr, "Could not open file '%s' for writing.\n", fname ) ;
    return FALSE ;
  }

  writeHeader () ;

  writeEntity ( root ) ;

  fclose ( save_fd ) ;
  return TRUE ;
}

