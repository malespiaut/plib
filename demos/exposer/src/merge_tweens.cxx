
#include "merge_tweens.h"

static void help ()
{
  fprintf ( stderr, "\n\n" ) ;
  fprintf ( stderr, "merge_tweens: Usage -\n\n" ) ;
  fprintf ( stderr, "    merge_tweens file.bones ...\n" ) ;
  fprintf ( stderr, "\n" ) ;
  fprintf ( stderr, "Merges multiple '.bones' files written from ExPoser\n" ) ;
  fprintf ( stderr, "into a single animation sequence (stored in 'out.bones').\n" ) ;
  fprintf ( stderr, "\n" ) ;
  fprintf ( stderr, "Bones files must be compatible (same number of joints\n" );
  fprintf ( stderr, "in same order, etc).\n" ) ;
  fprintf ( stderr, "\n" ) ;
}


static int   numBones     =   0  ;
static int   totalEvents  =   0  ;
static float totalTime    = 0.0f ;
static float floor_z      = 0.0f ;
static float ground_speed = 0.0f ;

int main ( int argc, char **argv )
{
  if ( argc < 2 )
  {
    help () ;
    exit ( 1 ) ;
  }

  /*
    First Pass - add up times - do error checking.
  */

  totalEvents =  0   ;
  totalTime   = 0.0f ;

  int i ;

  for ( i = 1 ; i < argc ; i++ )
  {
    FILE *fd = fopen ( argv[i], "ra" ) ;

    int bones, numevents ;
    float new_floor_z, maxtime, new_ground_speed ;
 
    fscanf ( fd, "NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
	      &bones, &numevents, &maxtime, &new_floor_z, &new_ground_speed ) ;

    printf (
      "%2d) %s: NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
       i, argv[i], bones, numevents, maxtime, new_floor_z, new_ground_speed ) ;
    
    totalEvents += numevents   ;
    totalTime   += maxtime     ;

    if ( i == 1 )
    {
      numBones     = bones       ;
      floor_z      = new_floor_z ;
      ground_speed = new_ground_speed ;
    }
    else
    {
      if ( numBones != bones )
      {
        fprintf ( stderr,
     "merge_bones: FATAL - Number of bones must be the same across files.\n" ) ;
        exit ( 1 ) ;
      }

      if ( floor_z != new_floor_z )
      {
        fprintf ( stderr,
            "merge_bones: WARNING - Floor Z is not the same across files.\n" ) ;
      }

      if ( ground_speed != new_ground_speed )
      {
        fprintf ( stderr,
         "merge_bones: WARNING - Ground speed is not the same across files.\n");
      }
    }

    fclose ( fd ) ;
  }

  /*
    Pass TWO : Write out composite file.
  */

  FILE *out_fd = fopen ( "out.bones", "wa" ) ;

  float nextTime = 0.0f ;

  for ( i = 1 ; i < argc ; i++ )
  {
    FILE *fd = fopen ( argv[i], "ra" ) ;

    int bones, numevents ;
    float new_floor_z, maxtime, new_ground_speed ;
 
    fscanf ( fd, "NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
	      &bones, &numevents, &maxtime, &new_floor_z, &new_ground_speed ) ;

    if ( i == 1 )
    {
      fprintf ( out_fd,
                 "NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
                 numBones, totalEvents, totalTime, floor_z, ground_speed ) ;
    
      for ( int j = 0 ; j < numBones ; j++ )
      {
        char s [ 1000 ] ;
        fgets ( s, 1000, fd ) ;
        fprintf ( out_fd, "%s", s ) ;
      }
    }
    else
    {
      for ( int j = 0 ; j < numBones ; j++ )
      {
        char s [ 1000 ] ;
        fgets ( s, 1000, fd ) ;
      }
    } 

    for ( int j = 0 ; j < numevents ; j++ )
    {
      float t, x, y, z ;
      int nb ;

      fscanf ( fd, "EVENT %f %d (%f,%f,%f)\n", &t, &nb, &x, &y, &z ) ;
      fprintf ( out_fd, "EVENT %f %d (%f,%f,%f)\n", t+nextTime, nb, x, y, z ) ;

      for ( int k = 0 ; k < nb ; k++ )
      {
        int xx, yy, zz ;
        fscanf ( fd, "  (%d,%d,%d)\n", &xx, &yy, &zz ) ;
        fprintf ( out_fd, "  (%d,%d,%d)\n", xx, yy, zz ) ;
      }
    }

    nextTime += maxtime ;
    fclose ( fd ) ;
  }

  fclose ( out_fd ) ;

  return 0 ;
}



