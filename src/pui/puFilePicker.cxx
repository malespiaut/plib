
#include "puLocal.h"

static void handle_slider ( puObject * slider )
{
  float val ;
  slider -> getValue ( &val ) ;
  val = 1.0f - val ;

  puListBox* list_box = (puListBox*) slider -> getUserData () ;
  int index = int ( list_box -> getNumItems () * val ) ;
  list_box -> setTopItem ( index ) ;
}

void puFilePicker::handle_select ( puObject* list_box )
{
  puFilePicker* file_picker = (puFilePicker*) list_box -> getUserData () ;

  int selected ;
  list_box -> getValue ( &selected ) ;

  if ( selected >= 0 && selected < file_picker -> num_files )
    file_picker -> setValue ( file_picker -> files [ selected ] ) ;
  else
    file_picker -> setValue ( "" ) ;
}

static void handle_cancel ( puObject* b )
{
  puFilePicker* file_picker = (puFilePicker*) b -> getUserData () ;
  file_picker -> setValue ( "" ) ;
  file_picker -> invokeCallback () ;
}

static void handle_ok ( puObject* b )
{
  puFilePicker* file_picker = (puFilePicker*) b -> getUserData () ;
  file_picker -> invokeCallback () ;
}

puFilePicker::puFilePicker ( int x, int y, const char* dir ) : puDialogBox ( x, y )
{
  files = 0 ;
  num_files = 0 ;
  setValue ( "" ) ;

  find_files ( dir ) ;

  new puFrame ( 0, 0, 220, 170 );

  puSlider* slider = new puSlider (170+10,40,100,TRUE);
  slider->setDelta(0.1f);
  slider->setValue(1.0f);
  slider->setSliderFraction (0.2f) ;
  slider->setCBMode( PUSLIDER_DELTA );
  
  puListBox* list_box = new puListBox ( 20, 40, 170, 140, files ) ;
  list_box -> setLabel ( "Pick a file" );
  list_box -> setLabelPlace ( PUPLACE_ABOVE ) ;
  list_box -> setStyle ( -PUSTYLE_SMALL_SHADED ) ;
  list_box -> setValue ( 0 ) ;

  list_box -> setUserData ( this ) ;
  list_box -> setCallback ( handle_select ) ;
  
  slider -> setUserData ( list_box ) ;
  slider -> setCallback ( handle_slider ) ;

  puOneShot* cancel_button = new puOneShot ( 20, 10, 90, 30 ) ;
  cancel_button -> setLegend ( "Cancel" ) ;
  cancel_button -> setUserData ( this ) ;
  cancel_button -> setCallback ( handle_cancel ) ;
  
  puOneShot* ok_button = new puOneShot ( 100, 10, 170, 30 ) ;
  ok_button -> setLegend ( "Ok" ) ;
  ok_button -> setUserData ( this ) ;
  ok_button -> setCallback ( handle_ok ) ;
//  ok_button->makeReturnDefault ( TRUE ) ;

  close  () ;
  reveal () ;
}

puFilePicker::~puFilePicker ()
{
  if ( files )
  {
    for ( int i=0; i<num_files; i++ )
    {
      delete files[i];
      files[i] = 0;
    }
    delete[] files;
    files = 0;
  }
  num_files = 0;
}

void puFilePicker::find_files ( const char* dir )
{
  num_files = 0;
  for ( int pass=0; pass<2; pass++ )
  {
    int ifile = 0;
    
    ulDir* dirp = ulOpenDir(dir);
    if ( dirp != NULL )
    {
      ulDirEnt* dp;
      while ( (dp = ulReadDir(dirp)) != NULL )
      {
        if ( ! dp->d_isdir )
        {
          if ( pass )
          {
            files[ ifile ] = new char[ strlen(dp->d_name)+1 ];
            strcpy( files[ ifile ], dp->d_name );
          }
          
          ifile ++;
        }
      }
      ulCloseDir(dirp);
    }
    
    if ( pass == 0 )
    {
      num_files = ifile;
      if ( num_files == 0 )
        return;
      
      files = new char* [ num_files+1 ];
      for ( int i=0; i<=num_files; i++ )
        files [i] = 0 ;
    }
  }
}
