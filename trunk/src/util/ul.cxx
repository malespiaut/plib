
#include "ul.h"

#ifndef WIN32
#  ifndef macintosh
#    include <GL/glx.h>
#  else
#    include <agl.h>
#  endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__CYGWIN__) || !defined(WIN32)
#include <dirent.h>
#endif


struct _ulDir
{
  char dirname [ UL_NAME_MAX+1 ];
  ulDirEnt curr ;

#if defined(WIN32) && !defined(__CYGWIN__)
  WIN32_FIND_DATA data ;
  HANDLE hFind ;
  bool first ;
  bool done ;
#else
  DIR* dirp ;
#endif
} ;


void ulInit ()
{
}


ulDir* ulOpenDir ( const char* dirname )
{
  ulDir* dir = new ulDir;
  if ( dir != NULL )
  {
    strcpy( dir->dirname, dirname ) ;
#if defined(WIN32) && !defined(__CYGWIN__)
    char search[_MAX_PATH];
    strcpy(search,dirname);
    
    //munch the directory seperator
    int len = strlen(search);
    if ( len>0 && strchr("/\\",search[len-1]) )
      search[len-1] = 0;
    
    //add the wildcard
    strcat(search,"/*.*");
    
    dir->first = true;
    dir->done = false;
    dir->hFind = FindFirstFile(search, &dir->data);
    if (dir->hFind == INVALID_HANDLE_VALUE)
    {
      delete dir;
      dir = NULL;
    }
#else
    dir->dirp = opendir(dirname) ;
#endif
  }
  return dir;
}


ulDirEnt* ulReadDir ( ulDir* dir )
{
  //read the next entry from the directory
#if defined(WIN32) && !defined(__CYGWIN__)
  //update state
  if ( dir->first )
    dir->first = false ;
  else if ( !dir->done && !FindNextFile(dir->hFind,&dir->data) )
    dir->done = true ;
  if ( dir->done )
    return NULL ;

  strcpy( dir->curr.d_name, dir->data.cFileName ) ;
#else
  struct dirent* direntp = readdir( dir->dirp );
  if ( !direntp )
    return NULL ;

  strcpy( dir->curr.d_name, direntp->d_name );
#endif

  char path[ UL_NAME_MAX+1 ];
  sprintf( path, "%s/%s", dir->dirname, dir->curr.d_name );

  //determine if this entry is a directory
#if defined(WIN32) && !defined(__CYGWIN__)
  struct _stat buf;
  int result = _stat(path,&buf);
  if ( result == 0 )
    dir->curr.d_isdir = (buf.st_mode & _S_IFDIR) != 0 ;
  else
    dir->curr.d_isdir = false ;
#else
  struct stat buf ;
  if ( stat(path,&buf) == 0 )
    dir->curr.d_isdir = (buf.st_mode & S_IFDIR) != 0 ;
  else
    dir->curr.d_isdir = false ;
#endif

  return( &dir->curr ) ;
}



void ulCloseDir ( ulDir* dir )
{
  if ( dir != NULL )
  {
#if defined(WIN32) && !defined(__CYGWIN__)
    FindClose(dir->hFind);
#else
    closedir(dir->dirp);
#endif
    delete dir;
  }
}

int ulGetCurrentContext ()
{
#ifdef WIN32
   if (wglGetCurrentContext () == NULL )
      return 0;
#else
#  if defined(macintosh)
      if (aglGetCurrentContext() == NULL )
         return 0;
#  else
      if (glXGetCurrentContext() == NULL )
         return 0
#  endif
#endif
return 1;
}
