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


#include <stdio.h>

enum pslResult
{
  PSL_PROGRAM_END,
  PSL_PROGRAM_PAUSE,
  PSL_PROGRAM_CONTINUE
} ;


typedef unsigned char pslOpcode ;
class pslContext ;
class pslCompiler  ;
class pslProgram ;

 
union pslVariable
{
  float f ;
  int   i ;
} ;                                                                             


class pslExtension
{
public:
  const char *symbol ;
  int   argc ;
  pslVariable (*func) ( int, pslVariable *, pslProgram *p ) ;
} ;



class pslProgram
{
  pslOpcode     *code       ;
  pslContext    *context    ;
  pslCompiler     *compiler     ;
  pslExtension  *extensions ;

  void *userData ;

public:

   pslProgram ( pslExtension *ext ) ;
   pslProgram ( pslProgram *src ) ;

  ~pslProgram () ;

  pslContext   *getContext     () const { return context    ; }
  pslOpcode    *getCode        () const { return code       ; }
  pslCompiler  *getCompiler    () const { return compiler     ; }
  pslExtension *getExtensions  () const { return extensions ; }

  void      *getUserData () const     { return userData ; }
  void       setUserData ( void *ud ) { userData = ud ; }

  void       dump  () const ;
  int        compile ( const char *fname ) ;
  int        compile ( FILE *fd ) ;
  void       reset () ;
  pslResult step  () ;
} ;


void pslInit () ;

