
#include "puLocal.h"

static void puFilePickerHandleSlider ( puObject * slider )
{
  float val ;
  slider -> getValue ( &val ) ;
  val = 1.0f - val ;

  puListBox* list_box = (puListBox*) slider -> getUserData () ;
  int index = int ( list_box -> getNumItems () * val ) ;
  list_box -> setTopItem ( index ) ;
}

static void puFilePickerHandleArrow ( puObject *arrow )
{
  puSlider *slider = (puSlider *) arrow->getUserData () ;
  puListBox* list_box = (puListBox*) slider -> getUserData () ;

  int type = ((puArrowButton *)arrow)->getArrowType() ;
  int inc = ( type == PUARROW_DOWN     ) ?   1 :
            ( type == PUARROW_UP       ) ?  -1 :
            ( type == PUARROW_FASTDOWN ) ?  10 :
            ( type == PUARROW_FASTUP   ) ? -10 : 0 ;

  float val ;
  slider -> getValue ( &val ) ;
  val = 1.0f - val ;
  int num_items = list_box->getNumItems () - 1 ;
  if ( num_items > 0 )
  {
    int index = int ( num_items * val + 0.5 ) + inc ;
    if ( index > num_items ) index = num_items ;
    if ( index < 0 ) index = 0 ;

    slider -> setValue ( 1.0f - (float)index / num_items ) ;
    list_box -> setTopItem ( index ) ;
  }
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

static void puFilePickerHandleCancel ( puObject* b )
{
  puFilePicker* file_picker = (puFilePicker*) b -> getUserData () ;
  file_picker -> setValue ( "" ) ;
  file_picker -> invokeCallback () ;
}

static void puFilePickerHandleOk ( puObject* b )
{
  puFilePicker* file_picker = (puFilePicker*) b -> getUserData () ;
  file_picker -> invokeCallback () ;
}

void puFilePicker::setSize ( int w, int h )
{
  puObject *ob ;
  for ( ob = dlist; ob != NULL; ob = ob->next )
  {
    if ( ob->getType() & PUCLASS_FRAME )  /* Resize the frame */
      ob->setSize ( w, h ) ;
    else if ( ob->getType() & PUCLASS_SLIDER )  /* Resize and position the slider */
    {
      ob->setPosition ( w-30, 40+20*arrow_count ) ;
      ob->setSize ( 20, h-70-40*arrow_count ) ;
    }
    else if ( ob->getType() & PUCLASS_ARROW )  /* Position the arrow buttons */
    {
      int type = ((puArrowButton *)ob)->getArrowType () ;
      if ( type == PUARROW_DOWN )
        ob->setPosition ( w-30, 20+20*arrow_count ) ;
      else if ( type == PUARROW_FASTDOWN )
        ob->setPosition ( w-30, 40 ) ;
      else if ( type == PUARROW_UP )
        ob->setPosition ( w-30, h-30-20*arrow_count ) ;
      else  /* fast up */
        ob->setPosition ( w-30, h-50 ) ;
    }
    else if ( ob->getType() & PUCLASS_LISTBOX )  /* Resize the list box */
      ob->setSize ( w-40, h-70 ) ;
    else  /* One-shot widgets, need to distinguish between them */
    {
      ob->setSize ( (w<170)?(w/2-15):70, 20 ) ;  /* Both buttons are the same size */
      if ( *( ob->getLegend () ) == 'O' )  /* "Ok" button */
        ob->setPosition ( (w<170)?(w/2+5):90, 10 ) ;
    }
  }
}

puFilePicker::puFilePicker ( int x, int y, int w, int h, int arrows, const char* dir, const char *title )
                           : puDialogBox ( x, y )
{
  puFilePickerInit ( x, y, w, h, arrows, dir, title ) ;
}

puFilePicker::puFilePicker ( int x, int y, int w, int h, const char* dir, const char *title )
                           : puDialogBox ( x, y )
{
  puFilePickerInit ( x, y, w, h, 1, dir, title ) ;
}

puFilePicker::puFilePicker ( int x, int y, int arrows, const char* dir, const char *title )
                           : puDialogBox ( x, y )
{
  puFilePickerInit ( x, y, arrows, 220, 170, dir, title ) ;
}

puFilePicker::puFilePicker ( int x, int y, const char* dir, const char *title )
                           : puDialogBox ( x, y )
{
  puFilePickerInit ( x, y, 220, 170, 1, dir, title ) ;
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

  if ( this == puActiveWidget () )
    puDeactivateWidget () ;
}

void puFilePicker::puFilePickerInit ( int x, int y, int w, int h, int arrows,
                                      const char *dir, const char *title )
{
  type |= PUCLASS_FILEPICKER ;
  files = 0 ;
  num_files = 0 ;
  setValue ( "" ) ;

  find_files ( dir ) ;

  if ( arrows > 2 ) arrows = 2 ;
  if ( arrows < 0 ) arrows = 0 ;
  arrow_count = arrows ;

  new puFrame ( 0, 0, w, h );

  puSlider* slider = new puSlider (w-30,40+20*arrows,h-70-40*arrows,TRUE,20);
  slider->setDelta(0.1f);
  slider->setValue(1.0f);
  slider->setSliderFraction (0.2f) ;
  slider->setCBMode( PUSLIDER_DELTA );
  
  puListBox* list_box = new puListBox ( 10, 40, w-40, h-30, files ) ;
  list_box -> setLabel ( title );
  list_box -> setLabelPlace ( PUPLACE_ABOVE ) ;
  list_box -> setStyle ( -PUSTYLE_SMALL_SHADED ) ;
  list_box -> setUserData ( this ) ;
  list_box -> setCallback ( handle_select ) ;
  list_box -> setValue ( 0 ) ;
  handle_select ( list_box ) ;
  
  slider -> setUserData ( list_box ) ;
  slider -> setCallback ( puFilePickerHandleSlider ) ;

  puOneShot* cancel_button = new puOneShot ( 10, 10, (w<170)?(w/2-5):80, 30 ) ;
  cancel_button -> setLegend ( "Cancel" ) ;
  cancel_button -> setUserData ( this ) ;
  cancel_button -> setCallback ( puFilePickerHandleCancel ) ;
  
  puOneShot* ok_button = new puOneShot ( (w<170)?(w/2+5):90, 10, (w<170)?(w-10):160, 30 ) ;
  ok_button -> setLegend ( "Ok" ) ;
  ok_button -> setUserData ( this ) ;
  ok_button -> setCallback ( puFilePickerHandleOk ) ;
//  ok_button->makeReturnDefault ( TRUE ) ;

  if ( arrows > 0 )
  {
    puArrowButton *down_arrow = new puArrowButton ( w-30, 20+20*arrows, w-10, 40+20*arrows, PUARROW_DOWN ) ;
    down_arrow->setUserData ( slider ) ;
    down_arrow->setCallback ( puFilePickerHandleArrow ) ;

    puArrowButton *up_arrow = new puArrowButton ( w-30, h-30-20*arrows, w-10, h-10-20*arrows, PUARROW_UP ) ;
    up_arrow->setUserData ( slider ) ;
    up_arrow->setCallback ( puFilePickerHandleArrow ) ;
  }

  if ( arrows == 2 )
  {
    puArrowButton *down_arrow = new puArrowButton ( w-30, 40, w-10, 60, PUARROW_FASTDOWN ) ;
    down_arrow->setUserData ( slider ) ;
    down_arrow->setCallback ( puFilePickerHandleArrow ) ;

    puArrowButton *up_arrow = new puArrowButton ( w-30, h-50, w-10, h-30, PUARROW_FASTUP ) ;
    up_arrow->setUserData ( slider ) ;
    up_arrow->setCallback ( puFilePickerHandleArrow ) ;
  }

  close  () ;
  reveal () ;
}

static int puFilePickerStringCompare ( const char *s1, const char *s2 )
{
  while ( 1 )
  {
    char c1 = s1? (*s1++): 0 ;
    char c2 = s2? (*s2++): 0 ;
    
    //end of string?
    if ( !c1 || !c2 )
    {
      if ( c1 )
        return 1 ; //s1 is longer
      if ( c2 )
        return -1 ; //s1 is shorter
      return 0 ;
    }
    
    if ( c1 == c2 )
      continue ;
    
    if ( c1 >= 'a' && c1 <= 'z' )
      c1 = c1 - ('a'-'A') ;
    
    if ( c2 >= 'a' && c2 <= 'z' )
      c2 = c2 - ('a'-'A') ;
    
    if ( c1 != c2 )
    {
      if ( c1 < c2 )
        return -1 ;
      return 1 ;
    }
  }
  return 0 ;
}

static void puFilePickerSort ( char** list, int size )
//
//  comb sort - a modified bubble sort
//    taken from BYTE, April 1991, ppg 315-320
//
{
  int switches;
  int gap = size;
  do
  {
    gap = ((gap * 197) >> 8);  // gap /= 1.3;
    switch (gap)
    {
    case 0:  // the smallest gap is 1 -- bubble sort
      gap = 1;
      break;
    case 9:  // this is what makes this Combsort11
    case 10:
      gap = 11;
      break;
    }
    switches = 0; // dirty pass flag
    int top = size - gap;
    for ( int i=0; i<top; ++i )
    {
      int j=i+gap;
      if (puFilePickerStringCompare(list[i],list[j]) > 0)
      {
        char* temp = list[i];
        list[i] = list[j];
        list[j] = temp;
        ++switches;
      }
    }
  }
  while(switches || gap>1);
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
  puFilePickerSort( files, num_files ) ;
}
