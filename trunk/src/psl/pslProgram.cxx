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

PSL_Program::PSL_Program ( PSL_Extension *ext )
{
  code = new PSL_Opcode [ MAX_CODE ] ;

  extensions = ext ;

  parser  = new PSL_Parser  ( code, ext ) ;
  context = new PSL_Context ( this ) ;

  parser  -> init  () ;
  context -> reset () ;
}
 

PSL_Program::PSL_Program ( PSL_Program *src )
{
  code       = src -> getCode       () ;
  parser     = src -> getParser     () ;
  extensions = src -> getExtensions () ;
  userData   = src -> getUserData   () ;

  context = new PSL_Context ( this ) ;
  context -> reset () ;
}
 

PSL_Program::~PSL_Program ()
{
  delete parser ;
  delete context ;
  delete [] code ;
}


void       PSL_Program::dump  () const {        parser  -> dump  () ; }
void       PSL_Program::reset ()       {        context -> reset () ; }
PSL_Result PSL_Program::step  ()       { return context -> step  () ; }

int        PSL_Program::parse ( const char *fname )
{
  return parser -> parse(fname) ;
}

int        PSL_Program::parse ( FILE *fd )
{
  return parser -> parse( fd  ) ;
}

