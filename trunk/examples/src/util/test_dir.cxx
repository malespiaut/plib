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
