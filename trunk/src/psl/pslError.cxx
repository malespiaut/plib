
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

static char _pslErrorBuffer [ 1024 ] = { '\0' } ;                     


void (*_pslErrorCB) ( int severity, const char *progname, int line_no,
                                    const char *message ) = NULL ;


void pslSetErrorCallback ( void (*CB) ( int, const char *, int, const char * ) )
{
  _pslErrorCB = CB ;
}



int pslCompiler::warning ( const char *fmt, ... )
{
  va_list argp;
  va_start ( argp, fmt ) ;
  vsprintf ( _pslErrorBuffer, fmt, argp ) ;
  va_end ( argp ) ;
 
  if ( _pslErrorCB != NULL )
    (*_pslErrorCB)( PSL_COMPILETIME_WARNING, getProgName(), getLineNo(), 
                                                            _pslErrorBuffer ) ;
  else
    fprintf ( stderr, "PSL: \"%s\" line %3d: WARNING - %s\n",
                                             getProgName(),getLineNo(), 
                                                           _pslErrorBuffer ) ;

  bumpWarnings () ;
  return FALSE ;
}



int pslCompiler::error ( const char *fmt, ... )
{
  va_list argp;
  va_start ( argp, fmt ) ;
  vsprintf ( _pslErrorBuffer, fmt, argp ) ;
  va_end ( argp ) ;
 
  if ( _pslErrorCB != NULL )
    (*_pslErrorCB)( PSL_COMPILETIME_ERROR, getProgName(), getLineNo(), 
                                                           _pslErrorBuffer ) ;
  else
    fprintf ( stderr, "PSL: \"%s\" line %3d: *ERROR* - %s\n",
                                             getProgName(),getLineNo(), 
                                                           _pslErrorBuffer ) ;

  bumpErrors () ;
  return FALSE ;
}



void pslContext::warning ( const char *fmt, ... )
{
  va_list argp;
  va_start ( argp, fmt ) ;
  vsprintf ( _pslErrorBuffer, fmt, argp ) ;
  va_end ( argp ) ;
 
  if ( _pslErrorCB != NULL )
    (*_pslErrorCB)( PSL_RUNTIME_WARNING, getProgName(), pc, _pslErrorBuffer ) ;
  else
    fprintf ( stderr, "PSL: \"%s\" PC=%d: WARNING - %s\n",
                                         getProgName(), pc, _pslErrorBuffer ) ;

  bumpWarnings () ;
}



void pslContext::error ( const char *fmt, ... )
{
  va_list argp;
  va_start ( argp, fmt ) ;
  vsprintf ( _pslErrorBuffer, fmt, argp ) ;
  va_end ( argp ) ;
 
  if ( _pslErrorCB != NULL )
    (*_pslErrorCB)( PSL_RUNTIME_ERROR, getProgName(), pc, _pslErrorBuffer ) ;
  else
    fprintf ( stderr, "PSL: \"%s\" PC=%d: *ERROR* - %s\n",
                                       getProgName(), pc, _pslErrorBuffer ) ;

  bumpErrors () ;
}


