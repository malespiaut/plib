/*
     This file is part of ExPoser - A Tool for Animating PLIB Critters.
     Copyright (C) 2001  Steve Baker

     ExPoser is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     ExPoser is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with ExPoser; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "exposer.h"

#define FILE_SELECTOR_ARGS  (640-320)/2,(480-270)/2,320,270,1


static float tweeninterval = 1.0f/8.0f ;

void  setTweenInterval ( float t ) { tweeninterval = t ;    }
float getTweenInterval ()          { return tweeninterval ; }


static puFileSelector *file_selector = NULL ;


static char lastModelFilePath [ PUSTRING_MAX ] ;
static char lastModelFileName [ PUSTRING_MAX ] ;

static void dismissDialogCB ( puObject * ) ;

static puButton *dialog_button  = NULL ;


static void initDialog ()
{
  if ( dialog_button != NULL )
    return ;

  dialog_button = new puButton    ( 250, 240, "" ) ;
  dialog_button -> setSize        ( 400, 40 ) ;
  dialog_button -> setLegendFont  ( PUFONT_TIMES_ROMAN_24 ) ;
  dialog_button -> setCallback    ( dismissDialogCB ) ;
  dialog_button -> setColorScheme ( 1, 1, 0, 1 ) ;
  dialog_button -> hide           () ;
}


static void dismissDialogCB ( puObject * )
{
  initDialog () ;
  dialog_button  -> hide () ;
}


static void dialog ( const char *msg, float r, float g, float b )
{
  initDialog () ;
  dialog_button -> setLegend ( msg ) ;
  dialog_button -> setColorScheme ( r, g, b, 1 ) ;
  dialog_button -> reveal () ;
}


static void twsavepickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;

  if ( path [ 0 ] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;

    dialog ( "FAILED TO SAVE TWEENED MODEL!", 1, 0, 0 ) ;
    return ;
  }

  saveTweenFile ( path, TRUE ) ;
}


void saveTweenFile ( char *path, int interactive )
{
  char orig_path [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;
  char *p = NULL ;
  int i ;

  strcpy ( orig_path, path ) ;

  for ( i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;
    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;
    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( path ) ;
    strcpy ( fname, p ) ;
  }

  /* SAVE THE TWEENED MODEL */

  if ( orig_path[0] == '\0' )
  {
    if ( interactive )
    {
      puDeleteObject ( file_selector ) ;
      file_selector = NULL ;
      dialog ( "FAILED TO SAVE TWEENED MODEL!", 1, 0, 0 ) ;
      return ;
    }
    else
    {
      perror ( "saveTween:" ) ;
      exit ( 1 ) ;
    }
  }

  tweenScene = (ssgRoot *) makeTweenCopy ( skinScene ) ;

  for ( i = 0 ; i <= (int)( timebox->getMaxTime() / tweeninterval ) ; i++ )
  {
    if ( i != 0 )
      addTweenBank ( tweenScene ) ;

    transformModel ( boneScene, (float) i * tweeninterval ) ;
    makeTweenCopy ( tweenScene, skinScene ) ;
  }

  if ( ! ssgSave ( orig_path, tweenScene ) )
  {
    if ( interactive )
    {
      puDeleteObject ( file_selector ) ;
      file_selector = NULL ;
      dialog ( "FAILED TO SAVE TWEENED MODEL!", 1, 0, 0 ) ;
      return ;
    }
    else
    {
      perror ( "saveTween:" ) ;
      exit ( 1 ) ;
    }
  }

  if ( interactive )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;
    dialog ( "TWEENED MODEL WAS SAVED OK.", 1, 1, 0 ) ;
  }
}


static void bnsavepickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;

  if ( path [ 0 ] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;

    dialog ( "FAILED TO SAVE BONES!", 1, 0, 0 ) ;
    return ;
  }

  char *p = NULL ;
  int i ;

  for ( i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;
    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;
    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( path ) ;
    strcpy ( fname, p ) ;
  }

  /* SAVE THE BONES */

  if ( file_selector->getStringValue()[0] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;
    dialog ( "FAILED TO SAVE BONES!", 1, 0, 0 ) ;
    return ;
  }

  FILE *fd = fopen ( file_selector->getStringValue(), "wa" ) ;

  puDeleteObject ( file_selector ) ;
  file_selector = NULL ;

  if ( fd == NULL )
  {
    dialog ( "FAILED TO SAVE BONES!", 1, 0, 0 ) ;
    return ;
  }

  fprintf ( fd, "NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
           getNumBones(), eventList->getNumEvents(), timebox->getMaxTime (),
           -ground->getZcoord(), timebox->getGroundSpeed() ) ;

  for ( i = 0 ; i < getNumBones () ; i++ )
    getBone ( i ) -> write ( fd ) ;

  eventList -> write ( fd ) ;

  fclose ( fd ) ;
  dialog ( "BONES WERE SAVED OK.", 1, 1, 0 ) ;
}


static void bnpickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 
  if ( path [ 0 ] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;
    return ;
  }

  loadBoneFile ( path, TRUE ) ;
}

void loadBoneFile ( char *path, int interactive )
{
  char orig_path [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;
  char *p = NULL ;
  int i ;

  strcpy ( orig_path, path ) ;

  for ( i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;
    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;
    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( path ) ;
    strcpy ( fname, p ) ;
  }

  /* LOAD THE BONES */

  if ( orig_path[0] == '\0' )
  {
    if ( interactive )
    {
      puDeleteObject ( file_selector ) ;
      file_selector = NULL ;
      return ;
    }
    else
    {
      perror ( "loadBones:" ) ;
      exit ( 1 ) ;
    }
  }

  FILE *fd = fopen ( orig_path, "ra" ) ;

  if ( fd == NULL )
  {
    if ( interactive )
    {
      puDeleteObject ( file_selector ) ;
      file_selector = NULL ;
      return ;
    }
    else
    {
      perror ( "loadBones:" ) ;
      exit ( 1 ) ;
    }
  }

  timebox->deleteAll () ;

  int numbones, numevents ;
  float floor_z_coord, maxtime, new_ground_speed ;

  fscanf ( fd,
	"NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
	&numbones, &numevents,
	&maxtime, &floor_z_coord, &new_ground_speed ) ;


  /* Don't use the floor_z_coord from the file. */
  /* ground -> setZcoord ( floor_z_coord ) ; */

  timebox->setMaxTime ( maxtime ) ;
  timebox->setGroundSpeed ( new_ground_speed ) ;

  if ( numbones != getNumBones () )
  {
    fprintf ( stderr,
      "Number of bones in model (%d) doesn't agree with number in bones file (%d)!\n", getNumBones (), numbones ) ;
    exit ( 1 ) ;
  }

  for ( i = 0 ; i < getNumBones () ; i++ )
    getBone ( i ) -> read ( fd ) ;

  eventList -> read ( numevents, fd ) ;

  fclose ( fd ) ;

  if ( interactive )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;
  }
}



static void scpickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 
  puDeleteObject ( file_selector ) ;
  file_selector = NULL ;

  if ( path [ 0 ] == '\0' )
    return ;

  if ( strlen ( path ) >= 6 && strcmp(&path[strlen(path)-6], ".bones" ) == 0 )
  {
    fprintf ( stderr, "I think you tried to load a BONES file as 3D model.\n");
    fprintf ( stderr, "Try again!\n");
    scloadCB ( NULL ) ;
    return ;
  }

  char *p = NULL ;

  for ( int i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;

    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;

    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( path ) ;

    strcpy ( fname, p ) ;
  }

  delete sceneScene ;
  sceneScene = new ssgRoot ;
  sceneScene -> addKid ( ssgLoad ( fname, NULL ) ) ;
}


static void pickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 
  puDeleteObject ( file_selector ) ;
  file_selector = NULL ;

  if ( path [ 0 ] == '\0' )
    return ;

  if ( strlen ( path ) >= 6 && strcmp(&path[strlen(path)-6], ".bones" ) == 0 )
  {
    fprintf ( stderr, "I think you tried to load a BONES file as 3D model.\n");
    fprintf ( stderr, "Try again!\n");
    loadCB ( NULL ) ;
    return ;
  }

  loadFile ( path, TRUE ) ;
}

void loadFile ( char *path, int interactive )
{
  char fname [ PUSTRING_MAX ] ;
  char *p = NULL ;

  for ( int i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;
    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;
    if ( getenv ( "EXPOSER_TEXTURE_PATH" ) != NULL )
      ssgTexturePath ( getenv ( "EXPOSER_TEXTURE_PATH" ) ) ;
    else
      ssgTexturePath ( path ) ;
    strcpy ( fname, p ) ;
  }

  strcpy ( lastModelFilePath, path ) ;
  strcpy ( lastModelFileName, fname ) ;

  skinScene -> addKid ( ssgLoad ( fname, NULL ) ) ;
  ssgFlatten  ( skinScene -> getKid ( 0 ) ) ;
  ssgStripify ( skinScene -> getKid ( 0 ) ) ;
  boneScene -> addKid ( extractBones ( skinScene ) ) ;

  extractVertices ( skinScene ) ;
  timebox->deleteAll () ;

  eventList -> newEvent ( 0.0f ) ;

  ground -> setZcoord ( getLowestVertexZ () ) ;
}


void twsaveCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( FILE_SELECTOR_ARGS,
		            lastModelFilePath, "Save Tweened Model As..." ) ;
    file_selector -> setCallback ( twsavepickfn ) ;

    char guess_fname [ PUSTRING_MAX ] ;
    strcpy ( guess_fname, lastModelFileName ) ;

    for ( int i = strlen ( guess_fname ) ; i >= 0 ; i-- )
      if ( guess_fname [ i ] == '.' )
      {
        guess_fname[i] = '\0' ;
        break ;
      }

    strcat ( guess_fname, "_tweened.ssg" ) ;
    file_selector -> setInitialValue ( guess_fname ) ;
  }
}


void bnsaveCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( FILE_SELECTOR_ARGS,
			            lastModelFilePath, "Save Bones As..." ) ;
    file_selector -> setCallback ( bnsavepickfn ) ;

    char guess_fname [ PUSTRING_MAX ] ;
    strcpy ( guess_fname, lastModelFileName ) ;

    for ( int i = strlen ( guess_fname ) ; i >= 0 ; i-- )
      if ( guess_fname [ i ] == '.' )
      {
        guess_fname[i] = '\0' ;
        break ;
      }

    strcat ( guess_fname, ".bones" ) ;
    file_selector -> setInitialValue ( guess_fname ) ;
  }
}


void bnloadCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( FILE_SELECTOR_ARGS,
			           lastModelFilePath, "Load Bones from..." ) ;
    file_selector -> setCallback ( bnpickfn ) ;

    /* Guess an initial filename from the model filename */

    char guess_fname [ PUSTRING_MAX ] ;
    strcpy ( guess_fname, lastModelFileName ) ;

    for ( int i = strlen ( guess_fname ) ; i >= 0 ; i-- )
      if ( guess_fname [ i ] == '.' )
      {
        guess_fname[i] = '\0' ;
        break ;
      }

    strcat ( guess_fname, ".bones" ) ;
    file_selector -> setInitialValue ( guess_fname ) ;
  }
}



void scloadCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( FILE_SELECTOR_ARGS,
                                         "", "Load Scenery from..." ) ;
    file_selector -> setCallback ( scpickfn ) ;
  }
}


void loadCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( FILE_SELECTOR_ARGS,
                                         "", "Load Model from..." ) ;
    file_selector -> setCallback ( pickfn ) ;
  }
}


