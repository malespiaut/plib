/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include <plib/ul.h>

void list_files( const char* dirname )
{
  static int level=-1;
  level++;
  
  ulDir* dirp = ulOpenDir(dirname);
  if ( dirp != NULL )
  {
    ulDirEnt* dp;
    while ( (dp = ulReadDir(dirp)) != NULL )
    {
      for ( int i=0; i<level; i++) printf(" ");
      printf("%s", dp->d_name );
      if ( dp->d_isdir )
        printf("/");
      printf("\n");

      if ( dp->d_isdir &&
        strcmp (dp->d_name, ".") &&
        strcmp (dp->d_name, ".."))
      {
        char path[ UL_NAME_MAX+1 ];
        strcpy (path, dirname);
        strcat (path, "/");
        strcat (path, dp->d_name);

        list_files( path );
      }
    }
    ulCloseDir(dirp);
  }
  level--;
}

int main(int argc, char* argv[])
{
  list_files(".");
  return 0;
}
