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


#include "pslLocal.h"

pslProgram::pslProgram ( pslExtension *ext )
{
  if ( ! _pslInitialised )
    ulSetError ( UL_FATAL,
          "PSL: You didn't call pslInit() before using PSL functions." ) ;

  code = new pslOpcode [ MAX_CODE ] ;

  extensions = ext ;

  parser  = new pslParser  ( code, ext ) ;
  context = new pslContext ( this ) ;

  parser  -> init  () ;
  context -> reset () ;
}
 

pslProgram::pslProgram ( pslProgram *src )
{
  code       = src -> getCode       () ;
  parser     = src -> getParser     () ;
  extensions = src -> getExtensions () ;
  userData   = src -> getUserData   () ;

  context = new pslContext ( this ) ;
  context -> reset () ;
}
 

pslProgram::~pslProgram ()
{
  delete parser ;
  delete context ;
  delete [] code ;
}


void       pslProgram::dump  () const {        parser  -> dump  () ; }
void       pslProgram::reset ()       {        context -> reset () ; }
pslResult  pslProgram::step  ()       { return context -> step  () ; }

int        pslProgram::parse ( const char *fname )
{
  return parser -> parse(fname) ;
}

int        pslProgram::parse ( FILE *fd )
{
  return parser -> parse( fd  ) ;
}

