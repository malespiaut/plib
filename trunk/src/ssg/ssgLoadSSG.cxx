// code for loading and writing .ssg files

#include "ssgLocal.h"

static int _ssgInstanceListLength = 0 ;
static ssgBase **_ssgInstanceList = NULL ;
static int _ssgNextInstanceKey = 0 ;

// used for reading only:
int _ssgFileVersionNumber = 0 ;

int _ssgGetNextInstanceKey ()
{
  return ++_ssgNextInstanceKey ;
}

ssgBase *_ssgGetFromList ( int key )
{

	if ( _ssgInstanceListLength <= key ) 
	{
		ulSetError ( UL_WARNING, "Invalid key encountered while reading a .ssg-file.\n") ; 
		return (ssgBase *) NULL ;
	}
	else
		return _ssgInstanceList[key] ;
}

void _ssgAddToList ( int key, ssgBase *b )
{
	if (key == 0 ) // wk: I dont think key 0 is permissible here.
		ulSetError ( UL_WARNING, "Key zero encountered while reading a .ssg-file.\n") ; 
	if ( _ssgInstanceListLength <= key )
  {
    int temp_length = _ssgInstanceListLength ;
    ssgBase **temp = _ssgInstanceList ;
    int new_length = (_ssgInstanceListLength * 2 < key) ? (key + 128) :
                                          (_ssgInstanceListLength * 2) ;
		if ( new_length == 0 )
			if (key == 0 ) // Did already warn user
				new_length = 10;
		assert ( new_length != 0 );
		_ssgInstanceListLength = new_length ;

    _ssgInstanceList = new ssgBase *[ new_length ] ;
    memset ( _ssgInstanceList, 0, new_length * sizeof(ssgBase *) ) ;

    if ( temp_length > 0 )
    {
      memcpy ( _ssgInstanceList, temp, temp_length * sizeof(ssgBase *) ) ;
      delete[] temp ;
    }
  }

  _ssgInstanceList [ key ] = b ;
}


ssgEntity *ssgLoadSSG ( const char *fname, const ssgLoaderOptions* options )
{
  delete[] _ssgInstanceList ;
  _ssgInstanceList = NULL ;
  _ssgInstanceListLength = 0 ;
	_ssgNextInstanceKey = 0 ;
  
  char filename [ 1024 ] ;

  if ( fname [ 0 ] != '/' &&
       _ssgModelPath != NULL &&
       _ssgModelPath [ 0 ] != '\0' )
  {
    strcpy ( filename, _ssgModelPath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, fname ) ;
  }
  else
    strcpy ( filename, fname ) ;

  FILE *fd = fopen ( filename, "rb" ) ;

  if ( fd == NULL )
  {
    perror ( filename ) ;
    ulSetError ( UL_WARNING, 
      "ssgLoadSSG: Failed to open '%s' for reading\n", filename ) ;
    return NULL ;
  }

  int magic ;
  int t ;
  ssgEntity *kid ;

  _ssgReadInt ( fd, & magic ) ;

  if ( ( magic & 0xFFFFFF00 ) != ( SSG_FILE_MAGIC_NUMBER & 0xFFFFFF00 ) )
  {
    if (((magic & 0x0000FF)>> 0)==((SSG_FILE_MAGIC_NUMBER & 0xFF000000)>>24) &&
        ((magic & 0x00FF00)>> 8)==((SSG_FILE_MAGIC_NUMBER & 0x00FF0000)>>16) &&
        ((magic & 0xFF0000)>>16)==((SSG_FILE_MAGIC_NUMBER & 0x0000FF00)>> 8) ) 
    	ulSetError ( UL_WARNING, "ssgLoadSSG: File appears to be byte swapped!\n" ) ;
    else
    	ulSetError ( UL_WARNING, "ssgLoadSSG: File has incorrect magic number!\n" ) ;

    return NULL ;
  }

  /*
    Save the old version number so we can do recursive loads
  */

  int oldFileVersion = _ssgFileVersionNumber ;

  _ssgFileVersionNumber = ( magic & 0xFF ) ;


  if ( _ssgFileVersionNumber == 0 )
  {
    ulSetError ( UL_WARNING, 
            "ssgLoadSSG: SSG file format version zero is no longer supported, sorry! For more, see the docs.\n" ) ;
    _ssgFileVersionNumber = oldFileVersion ;
    return NULL ;
  }
	if ( _ssgFileVersionNumber > SSG_FILE_VERSION )
  {
    ulSetError ( UL_WARNING, 
            "ssgLoadSSG: This version of SSG is too old to load this file!\n" ) ;
    _ssgFileVersionNumber = oldFileVersion ;
    return NULL ;
  }

  _ssgReadInt ( fd, & t ) ;

  if ( t == ssgTypeVTable       () ) kid = new ssgVTable       () ; else
  if ( t == ssgTypeVtxTable     () ) kid = new ssgVtxTable     () ; else
  if ( t == ssgTypeVtxArray     () ) kid = new ssgVtxArray     () ; else
  if ( t == ssgTypeBranch       () ) kid = new ssgBranch       () ; else
  if ( t == ssgTypeTransform    () ) kid = new ssgTransform    () ; else
  if ( t == ssgTypeTexTrans     () ) kid = new ssgTexTrans     () ; else
  if ( t == ssgTypeSelector     () ) kid = new ssgSelector     () ; else
  if ( t == ssgTypeRangeSelector() ) kid = new ssgRangeSelector() ; else
  if ( t == ssgTypeTimedSelector() ) kid = new ssgTimedSelector() ; else
  if ( t == ssgTypeCutout       () ) kid = new ssgCutout       () ; else
  if ( t == ssgTypeInvisible    () ) kid = new ssgInvisible    () ; else
  if ( t == ssgTypeRoot         () ) kid = new ssgRoot         () ; else
  {
    ulSetError ( UL_WARNING, 
      "loadSSG: Unrecognised Entity type 0x%08x\n", t ) ;
    _ssgFileVersionNumber = oldFileVersion ;
    return NULL ;
  }

  if ( ! kid -> load ( fd ) )
  {
    ulSetError ( UL_WARNING, 
      "loadSSG: Failed to read child object.\n" ) ;
    _ssgFileVersionNumber = oldFileVersion ;
    return NULL ;
  }

  kid -> recalcBSphere () ;

  fclose ( fd ) ;
  _ssgFileVersionNumber = oldFileVersion ;
  return kid ;
}


int ssgSaveSSG ( const char *fname, ssgEntity *ent )
{
  /* Uses the spare field in every entity to make sure
    we don't save the same thing twice */

  _ssgNextInstanceKey = 0 ;
  ent -> zeroSpareRecursive () ;

  char filename [ 1024 ] ;

  if ( fname [ 0 ] != '/' &&
       _ssgModelPath != NULL &&
       _ssgModelPath [ 0 ] != '\0' )
  {
    strcpy ( filename, _ssgModelPath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, fname ) ;
  }
  else
    strcpy ( filename, fname ) ;

  FILE *fd = fopen ( filename, "wb" ) ;

  if ( fd == NULL )
  {
    perror ( filename ) ;
    ulSetError ( UL_WARNING, 
      "ssgSaveSSG: Failed to open '%s' for writing\n", filename ) ;
    return FALSE ;
  }

  _ssgWriteInt ( fd, SSG_FILE_MAGIC_NUMBER ) ;

  
	
	
	_ssgWriteInt ( fd, ent->getType() ) ;

	if ( ! ent -> save ( fd ) )
	{
		ulSetError ( UL_WARNING, 
    	"ssgSaveSSG: Failed to write child object.\n" ) ;
		return FALSE ;
	}
  fclose ( fd ) ;
  return TRUE ;
}


