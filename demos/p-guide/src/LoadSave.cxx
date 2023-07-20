/*
     This file is part of P-GUIDE -
     PUI-based Graphical User Interface Designer.
     Copyright (C) 2002, 2006  John F. Fay

     P-GUIDE is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     P-GUIDE is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with P-GUIDE; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     $Id$
*/


#include <stdio.h>

#include "WidgetList.h"

#define TAG_SAVEFILE        0x00000001
#define TAG_WINDOW          0x00000002
#define TAG_OBJECT          0x00000004
#define TAG_WIDGET          0x00000008
#define TAG_SIZE            0x00000010
#define TAG_POSITION        0x00000020
#define TAG_COLOR           0x00000040
#define TAG_LEGEND          0x00000080
#define TAG_LABEL           0x00000100
#define TAG_DATA            0x00000120

#define CALLBACK_UP         0x00000001
#define CALLBACK_ACTIVE     0x00000002
#define CALLBACK_DOWN       0x00000004

#ifdef WIN32
  #define PATH_SEPARATOR   '\\'
#else
  #define PATH_SEPARATOR   '/'
#endif

// From the Main Window:

extern WidgetList *widgets ;
extern int max_layer ;

extern int main_window_width  ;
extern int main_window_height ;

extern char main_window_name [ PUSTRING_MAX ] ;

extern int main_window_x ;
extern int main_window_y ;

extern int widget_number ;

extern bool main_window_changed ;
extern bool currently_loading ; /* Var for the Reshape function to not do its thing */
extern char pguide_current_directory [ PUSTRING_MAX ] ;

extern bool autolock ;


extern float main_window_color_r, main_window_color_g,
             main_window_color_b, main_window_color_a ;

// From the Status Window

extern void setStatusWidgets ( WidgetList *wid );
extern int status_window ;
extern int main_window ;
extern puaFileSelector *file_selector ;

// Duplication checking
static void chk_dupname ( WidgetList *new_wid ) ;
static puInput *dup_newname = (puInput *)NULL ;
static puDialogBox *dup_dialog = (puDialogBox *)NULL ;
static WidgetList *nextwid ;

// Now our turn

void saveProject ( puObject *ob ) {
    char* filename ;
      file_selector -> getValue ( &filename ) ;
    if (filename[0] == '\0')
    {
        puDeleteObject ( file_selector ) ;
        file_selector = (puaFileSelector *)NULL ;
        glutHideWindow () ;
        glutSetWindow ( status_window ) ;
        return ;
    }        

    /* Save the new current directory */
    strcpy(pguide_current_directory, filename) ;
    int i = strlen(pguide_current_directory);
    while (pguide_current_directory[i] != PATH_SEPARATOR) {
        if (i>0) i-- ;
        else break ;
    }
    pguide_current_directory[i+1] = '\0' ;

    /* If they didn't give an extension, then tack ".xml" onto the end. */
    if(!strstr(filename, "."))
        sprintf(filename, "%s.xml", filename);

    FILE *out = fopen ( filename, "wt" ) ;
    if ( !out )
     printf ( "ERROR opening file <%s> for writing\n", filename ) ;

    puDeleteObject ( file_selector ) ;
    file_selector = (puaFileSelector *)NULL ;
    glutHideWindow () ;
    glutSetWindow ( status_window ) ;

    if ( !out ) return ;

  /* Begin writing the XML */

    const char *autolock_text;

    if ( !autolock ) 
        autolock_text="FALSE";
    else
        autolock_text="TRUE";

    char projectname [] = "Test Project";

    fprintf ( out, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n<!DOCTYPE P-Guide>\n<savefile>\n\n" ) ;
    fprintf ( out, "<version>VERSION</version>\n") ;
    fprintf ( out, "<project>%s</project>\n", projectname) ;
    fprintf ( out, "<autolock>%s</autolock>\n", autolock_text) ;
    fprintf ( out, "<window>\n\t<name>%s</name>\n", main_window_name) ;
    fprintf ( out, "\t<size>\n\t\t<width>%d</width>\n", main_window_width) ;
    fprintf ( out, "\t\t<height>%d</height>\n\t</size>\n", main_window_height) ;
    fprintf ( out, "\t<position>\n\t\t<x>%d</x>\n", main_window_x) ;
    fprintf ( out, "\t\t<y>%d</y>\n\t</position>\n", main_window_y) ;
    fprintf ( out, "\t<color>\n\t\t<r>%f</r>\n\t\t<g>%f</g>\n\t\t<b>%f</b>\n", main_window_color_r, main_window_color_g, main_window_color_b) ;
    fprintf ( out, "\t\t<a>%f</a>\n\t</color>\n</window>\n\n", main_window_color_a) ;
    fprintf ( out, "<objects>\n") ;
    /* Now begin spitting out the actual widget data. */

    WidgetList *wid = widgets ;
    const char *place_name [] = { "PUPLACE_TOP_LEFT", "PUPLACE_TOP_CENTERED", "PUPLACE_TOP_RIGHT",
                                  "PUPLACE_CENTERED_LEFT", "PUPLACE_CENTERED_RIGHT",
                                  "PUPLACE_BOTTOM_LEFT", "PUPLACE_BOTTOM_CENTERED", "PUPLACE_BOTTOM_RIGHT",
                                  "PUPLACE_CENTERED_CENTERED",
                                  "PUPLACE_ABOVE_LEFT", "PUPLACE_ABOVE_RIGHT",
                                  "PUPLACE_BELOW_LEFT", "PUPLACE_BELOW_RIGHT",
                                  "PUPLACE_UPPER_LEFT", "PUPLACE_UPPER_RIGHT",
                                  "PUPLACE_LOWER_LEFT", "PUPLACE_LOWER_RIGHT" } ;


    while ( wid )
    {
        puObject *obj = wid->obj ;
        int x, y, w, h ;
        const char *hidden ;
        const char *locked ;
        const char *bool_one ;
        const char *bool_two ;
        const char *bool_three ;
        obj->getPosition ( &x, &y ) ;
        obj->getSize ( &w, &h ) ;

        fprintf ( out, "\t<widget>\n\t\t<name>%s</name>\n", wid->object_name) ;
        if (wid->legend_text != NULL)
            fprintf ( out, "\t\t<legend>\n\t\t\t<text>%s</text>\n\t\t\t<pos>%s</pos>\n\t\t</legend>\n", wid->legend_text, place_name [ obj->getLegendPlace () ] ) ;

        if (wid->label_text != NULL)
            fprintf ( out, "\t\t<label>\n\t\t\t<text>%s</text>\n\t\t\t<pos>%s</pos>\n\t\t</label>\n", wid->label_text, place_name [ obj->getLabelPlace () ] ) ;

        fprintf ( out, "\t\t<type>%s</type>\n", wid->object_type_name) ;

        if ( !wid->visible ) 
            hidden="FALSE";
        else
            hidden="TRUE";

        if ( !wid->locked ) 
            locked="FALSE";
        else
            locked="TRUE";

        if ( wid->callbacks & CALLBACK_UP )  // Up-callback defined
          fprintf ( out, "\t\t<callbacks>Up</callbacks>\n") ;
        if ( wid->callbacks & CALLBACK_ACTIVE )  // Active callback defined
          fprintf ( out, "\t\t<callbacks>Active</callbacks>\n") ;
        if ( wid->callbacks & CALLBACK_DOWN )  // Down-callback defined
          fprintf ( out, "\t\t<callbacks>Down</callbacks>\n") ;
        if ( wid->callbacks == 0 )
          fprintf ( out, "\t\t<callbacks></callbacks>\n") ;

        fprintf ( out, "\t\t<layer>%d</layer>\n", wid->layer) ;

        fprintf ( out, "\t\t<visible>%s</visible>\n", hidden) ;
        fprintf ( out, "\t\t<locked>%s</locked>\n", locked) ;
        fprintf ( out, "\t\t<size>\n\t\t\t<width>%d</width>\n\t\t\t<height>%d</height>\n\t\t</size>\n", w, h) ;
        fprintf ( out, "\t\t<position>\n\t\t\t<x>%d</x>\n\t\t\t<y>%d</y>\n\t\t</position>\n", x, y) ;

        fprintf ( out, "\t\t<data>\n\t\t\t<int1>%d</int1>\n\t\t\t<int2>%d</int2>\n", wid->intval1, wid->intval2) ;
        if ( wid->boolval1 )
            bool_one = "TRUE";
        else
            bool_one = "FALSE";

        if ( wid->boolval2 )
            bool_two = "TRUE";
        else
            bool_two = "FALSE";

        if ( wid->boolval3 )
            bool_three = "TRUE";
        else
            bool_three = "FALSE";

        fprintf ( out, "\t\t\t<bool1>%s</bool1>\n\t\t\t<bool2>%s</bool2>\n\t\t\t<bool3>%s</bool3>\n", bool_one, bool_two, bool_three) ;

        fprintf ( out, "\t\t\t<float1>%f</float1>\n\t\t\t<float2>%f</float2>\n", wid->floatval1, wid->floatval2) ;
        fprintf ( out, "\t\t\t<float3>%f</float3>\n\t\t\t<float4>%f</float4>\n", wid->floatval3, wid->floatval4) ;
        fprintf ( out, "\t\t\t<float5>%f</float5>\n\t\t\t<float6>%f</float6>\n", wid->floatval5, wid->floatval6) ;

        if (wid->allowed)
            fprintf ( out, "\t\t\t<allowed>%s</allowed>\n", wid->allowed) ;

        /* Walk through the list of items, and make a seperate <items> entry for each line in the string */
        if (wid->items)
        {
            char *temp_items = wid->items ;
            char *cr = strchr(temp_items, '\n');
            while ( cr )
            {
                *cr = '\0';
                fprintf ( out, "\t\t\t<item>%s</item>\n", temp_items);
                *cr = '\n';
                temp_items = cr + 1;
                cr = strchr(temp_items, '\n');
            }
            if (wid->object_type == PUCLASS_ARROW)
            {
                fprintf ( out, "\t\t\t<item>%s</item>\n", temp_items);
            }
        }
        
        fprintf ( out, "\t\t</data>\n\t</widget>\n") ;
        wid = wid->next ;
    }

    fprintf ( out, "</objects>\n</savefile>\n") ; /* XML output done */
    fclose (out) ;
}

/*static void gotoNextLine( char* buffer )
{
    while (buffer != "\n")
        buffer++ ;
}*/

void loadProject ( puObject *ob ) {
    extern puInput *window_name ;
    extern puInput *window_size_x ;
    extern puInput *window_size_y ;
    extern puInput *window_position_x ;
    extern puInput *window_position_y ;

    extern puaSpinBox *window_color_r ;
    extern puaSpinBox *window_color_g ;
    extern puaSpinBox *window_color_b ;
    extern puaSpinBox *window_color_a ;

    char* filename ;
    file_selector -> getValue ( &filename ) ;
    if (filename[0] == '\0')
    {
        puDeleteObject ( file_selector ) ;
        file_selector = (puaFileSelector *)NULL ;
        glutHideWindow () ;
        glutSetWindow ( status_window ) ;
        return ;
    }        

    /* Save the new current directory */
    strcpy(pguide_current_directory, filename) ;
    int i = strlen(pguide_current_directory);
    while (pguide_current_directory[i] != PATH_SEPARATOR) {
        if (i>0) i-- ;
        else break ;
    }
    pguide_current_directory[i+1] = '\0' ;

    FILE *in = fopen ( filename, "rt" ) ;
    if ( !in )
        printf ( "ERROR opening file <%s> for reading\n", filename ) ;

    puDeleteObject ( file_selector ) ;
    file_selector = (puaFileSelector *)NULL ;
    glutHideWindow () ;
    glutSetWindow ( status_window ) ;
    
    if ( !in ) return ;

    char rawbuffer[2048];
    // Verify file is XML

    float tver = 0.0f ;
    char encversion[PUSTRING_MAX];
    fgets( rawbuffer, sizeof(rawbuffer), in) ;
    sscanf(rawbuffer,"<?xml version=\"%f\" encoding=\"%s\" ?>", &tver, encversion) ;
    if (!tver) {
        printf ("File does not contain proper XML headings.\n");
        return ;
    }

    // Verify file is P-Guide
    char tag[PUSTRING_MAX] ;
    char tagvalue[PUSTRING_MAX] ;
    fgets( rawbuffer, sizeof(rawbuffer), in) ;

    if (!sscanf(rawbuffer,"<!DOCTYPE %s>", tag)) {
        printf ("File, though XML, does not contain P-Guide information (or has damaged header information).\n");
        return ;
    }

    // Start reading XML
    main_window_changed = true ;
    currently_loading = true ;
    int groupid = 0 ;
    puObject *new_obj = NULL;
    WidgetList *new_wid = NULL ;
    /* Dandy little variables */
    int wid_val_y = 0, wid_val_x = 0, val_object_width = 20, val_object_height = 20, val_label_place = 4, val_legend_place = 8;
    int intval1 = 0, intval2 = 0 ;
    bool boolval1 = false, boolval2 = false, boolval3 = false ;
    float floatval1 = 0.0f, floatval2 = 0.0f, floatval3 = 0.0f, floatval4 = 0.0f, floatval5 = 0.0f, floatval6 = 0.0f;

    while(fgets( rawbuffer, sizeof(rawbuffer), in))
    {
        /**************************************************************/
        /* Key for Groups in our XML file located at top of this file */
        /**************************************************************/

        *tag = '\0';
        *tagvalue = '\0';
        char *buffer = rawbuffer;

        /* From here on we walk, line by line, through the XML and parse any tags we come to. */
        while (*buffer == '\t' || *buffer == ' ') /* Ignore the tabs and spaces */
            buffer++ ;
        sscanf(buffer,"<%[^>]>", tag); /* Get the tag */

        while (*buffer !='>') buffer++;
    
        sscanf(buffer,">%[a-zA-Z_!@#$^&*0123456789-+/\\{}()=~`,. ]</", tagvalue) ; /* Get the tag's value. */

        /* Classify the tag, so we know where the next will fit. */
        if ( strstr(tag,"savefile") )   groupid |= TAG_SAVEFILE ;
        if ( strstr(tag,"window") )     groupid |= TAG_WINDOW ;
        if ( strstr(tag,"size") )       groupid |= TAG_SIZE ;
        if ( strstr(tag,"position") )   groupid |= TAG_POSITION ;
        if ( strstr(tag,"color") )      groupid |= TAG_COLOR ;
        if ( strstr(tag,"object") )     groupid |= TAG_OBJECT ;                
        if ( strstr(tag,"widget") )     groupid |= TAG_WIDGET ;
        if ( strstr(tag,"legend") )     groupid |= TAG_LEGEND ;
        if ( strstr(tag,"label") )      groupid |= TAG_LABEL ;
        if ( strstr(tag,"data") )       groupid |= TAG_DATA ;

        /* Note: Order here matters, since the strstrs above WILL match closing tags. This */
        /* doesn't hurt anything, as long as we remove the bitset when we realize they are */ 
        /* close tags, not open ones.                                                      */

        /* De-classify the tag, so we know where it doesn't fit. */
        if ( strstr(tag,"/savefile") )  groupid ^= TAG_SAVEFILE ;
        if ( strstr(tag,"/window") )    groupid ^= TAG_WINDOW ;
        if ( strstr(tag,"/size") )      groupid ^= TAG_SIZE ;
        if ( strstr(tag,"/position") )  groupid ^= TAG_POSITION ;
        if ( strstr(tag,"/color") )     groupid ^= TAG_COLOR ;
        if ( strstr(tag,"/object") )    groupid ^= TAG_OBJECT ;                
        if ( strstr(tag,"/widget") )    groupid ^= TAG_WIDGET ;
        if ( strstr(tag,"/legend") )    groupid ^= TAG_LEGEND ;
        if ( strstr(tag,"/label") )     groupid ^= TAG_LABEL ;
        if ( strstr(tag,"/data") )      groupid ^= TAG_DATA ;

        /* Now that the tag's classifcation has been determined, rip the data */
        /* viciously from its' hold and throw it onto the memory stack.       */
 
        if (groupid & TAG_SAVEFILE )
        {
            /* NULLCHECK! */
            if (tagvalue[0] == '\0') { sprintf(tagvalue, " "); }
            if ( strstr(tag,"autolock") )
            {
                if (strstr(tagvalue, "TRUE"))
                    autolock = true ;
                else
                    autolock = false ;
            }
            if (groupid & TAG_WINDOW) 
            {
               if ( strstr(tag,"name") )
               {
                    window_name->setValue(tagvalue) ;
                    strcpy(main_window_name, tagvalue);
               }
               if (groupid & TAG_SIZE)
               {
                   if ( strstr(tag,"width") )
                       window_size_x->setValue(tagvalue) ;
                   if ( strstr(tag,"height") )
                       window_size_y->setValue(tagvalue) ;
               }
               if (groupid & TAG_POSITION)
               {
                   if ( strstr(tag,"x") )
                       window_position_x->setValue(tagvalue) ;
                   if ( strstr(tag,"y") )
                       window_position_y->setValue(tagvalue) ;
               }
               if (groupid & TAG_COLOR)
               {
                   if ( strstr(tag,"r") )
                       window_color_r->setValue(tagvalue) ;
                   if ( strstr(tag,"g") )
                       window_color_g->setValue(tagvalue) ;
                   if ( strstr(tag,"b") )
                       window_color_b->setValue(tagvalue) ;
                   if ( strstr(tag,"a") )
                       window_color_a->setValue(tagvalue) ;
               }
            }
            if (groupid & TAG_OBJECT) 
            {
               if ( strstr(tag,"/widget") )
               {
                   extern WidgetList *widgets ;
                   /* Create a new frame in the viewing window (for manipulation). */
                   new_obj = new puFrame ( 0, 0, val_object_width, val_object_height ) ;
                   new_obj->setWindow( main_window ) ;
                   /* Set the representative widget's position */
                   new_obj->setPosition ( wid_val_x, wid_val_y ) ;
                   new_obj->setLegend ( new_wid->object_type_name ) ;
                   new_obj->setLegendPlace ( val_legend_place ) ;
                   if (new_wid->label_text)
                   {
                        new_obj->setLabel ( new_wid->label_text ) ;
                        new_obj->setLabelPlace ( val_label_place ) ; /**/
                   }

                   new_wid->boolval1 = boolval1;
                   new_wid->boolval2 = boolval2;
                   new_wid->boolval3 = boolval3;
                   new_wid->intval1 = intval1;
                   new_wid->intval2 = intval2;
                   new_wid->floatval1 = floatval1;
                   new_wid->floatval2 = floatval2;
                   new_wid->floatval3 = floatval3;
                   new_wid->floatval4 = floatval4;
                   new_wid->floatval5 = floatval5;
                   new_wid->floatval6 = floatval6;

                   intval1 = intval2 = 0 ;
                   boolval1 = boolval2 = boolval3 = false ;
                   floatval1 = floatval2 = floatval3 = floatval4 = floatval5 = floatval6 = 0.0f;
                   new_wid->obj = new_obj ;
                   new_wid->next = widgets ;
                   widgets = new_wid ;
                   
               } else if ( strstr(tag,"widget") ) {
                   /* Add the new object to the list of widgets (for the builder itself) */
                   new_wid = new WidgetList ;
                   
                   /* Set those dandy variables */
                   wid_val_y = 0; wid_val_x = 0;
                   val_object_width = 20; val_object_height = 20;
                   val_label_place = 4; val_legend_place = 8;
                   new_wid->callbacks = 0;
                   new_wid->allowed = (char *)NULL ;
                   new_wid->items = (char *)NULL ;
                   new_wid->locked = false;
                   new_wid->legend_text = NULL ;
                   new_wid->label_text = NULL ;
               }


               if (groupid & TAG_WIDGET)
               {
                    if ( strstr(tag,"name") )
                    {
                        strcpy(new_wid->object_name, tagvalue) ;
                        /* As names are copied, look for defaults (widget0..9999), */
                        /* and if it's higher than the current widget_number, set  */
                        /* widget_number to it.                                    */
                        int tmp_wid_num = 0;
                        sscanf(tagvalue, "widget%d", &tmp_wid_num) ;
                        if (tmp_wid_num >= widget_number) widget_number = tmp_wid_num + 1;
                    }
                    if ( strstr(tag,"type") )
                    {
                        new_wid->object_type = 0 ;
                        new_wid->object_type_name = new char [strlen(tagvalue)+1] ;
                        strcpy(new_wid->object_type_name, tagvalue) ;
                        /* BEWARE: Popup[menu], BiSlider[WithEnds], and Button[box] want to be each other! Don't let them! */

                        if ( strstr(new_wid->object_type_name, "puaList"))             new_wid->object_type |= PUCLASS_LIST             ;
                        if ( strstr(new_wid->object_type_name, "puaChooser"))          new_wid->object_type |= PUCLASS_CHOOSER          ;
                        if ( strstr(new_wid->object_type_name, "puaSliderWithInput"))  new_wid->object_type |= PUCLASS_SLIDERWITHINPUT  ;
                        if ( strstr(new_wid->object_type_name, "puaBiSliderWithEnds")) new_wid->object_type |= PUCLASS_BISLIDERWITHENDS ;
                        else if ( strstr(new_wid->object_type_name, "puaBiSlider"))    new_wid->object_type |= PUCLASS_BISLIDER         ;
                        if ( strstr(new_wid->object_type_name, "puaScrollBar"))        new_wid->object_type |= PUCLASS_SCROLLBAR        ;
                        if ( strstr(new_wid->object_type_name, "puaSpinBox"))          new_wid->object_type |= PUCLASS_SPINBOX          ; /**/
                        if ( strstr(new_wid->object_type_name, "puaSelectBox"))        new_wid->object_type |= PUCLASS_SELECTBOX        ;
                        if ( strstr(new_wid->object_type_name, "puaComboBox"))         new_wid->object_type |= PUCLASS_COMBOBOX         ; 
                        if ( strstr(new_wid->object_type_name, "puaLargeInput"))       new_wid->object_type |= PUCLASS_LARGEINPUT       ;
                        if ( strstr(new_wid->object_type_name, "puaVerticalMenu"))     new_wid->object_type |= PUCLASS_VERTMENU         ;
                        if ( strstr(new_wid->object_type_name, "puaTriSlider"))        new_wid->object_type |= PUCLASS_TRISLIDER        ;
                        if ( strstr(new_wid->object_type_name, "puaFileSelector"))     new_wid->object_type |= PUCLASS_FILESELECTOR     ;
                        if ( strstr(new_wid->object_type_name, "puDial"))              new_wid->object_type |= PUCLASS_DIAL             ;
                        if ( strstr(new_wid->object_type_name, "puListBox"))           new_wid->object_type |= PUCLASS_LISTBOX          ;
                        if ( strstr(new_wid->object_type_name, "puArrowButton"))       new_wid->object_type |= PUCLASS_ARROW            ;
                        if ( strstr(new_wid->object_type_name, "puDialogBox"))         new_wid->object_type |= PUCLASS_DIALOGBOX        ; /**/
                        if ( strstr(new_wid->object_type_name, "puSlider"))            new_wid->object_type |= PUCLASS_SLIDER           ;
                        if ( strstr(new_wid->object_type_name, "puButtonBox"))         new_wid->object_type |= PUCLASS_BUTTONBOX        ;
                        else if ( strstr(new_wid->object_type_name, "puButton"))       new_wid->object_type |= PUCLASS_BUTTON           ; /* See above for why the elses' */
                        if ( strstr(new_wid->object_type_name, "puInput"))             new_wid->object_type |= PUCLASS_INPUT            ;
                        if ( strstr(new_wid->object_type_name, "puMenuBar"))           new_wid->object_type |= PUCLASS_MENUBAR          ;
                        if ( strstr(new_wid->object_type_name, "puPopupMenu"))         new_wid->object_type |= PUCLASS_POPUPMENU        ; /**/
                        else if ( strstr(new_wid->object_type_name, "puPopup"))        new_wid->object_type |= PUCLASS_POPUP            ; /**/
                        if ( strstr(new_wid->object_type_name, "puOneShot"))           new_wid->object_type |= PUCLASS_ONESHOT          ;
                        if ( strstr(new_wid->object_type_name, "puText"))              new_wid->object_type |= PUCLASS_TEXT             ;
                        if ( strstr(new_wid->object_type_name, "puFrame"))             new_wid->object_type |= PUCLASS_FRAME            ;
                    }
                    if ( strstr(tag,"callbacks") )
                    {
                        /* Update callback's bit set */
                        if (strstr(tagvalue, "Up"))        new_wid->callbacks |= CALLBACK_UP     ;
                        if (strstr(tagvalue, "Down"))      new_wid->callbacks |= CALLBACK_DOWN   ;
                        if (strstr(tagvalue, "Active"))    new_wid->callbacks |= CALLBACK_ACTIVE ;
                    }
                    if ( strstr(tag,"layer") )
                    {
                        new_wid->layer = atoi(tagvalue) ;
                        if ( max_layer <= new_wid->layer ) max_layer = new_wid->layer + 1 ;
                    }
                    if ( strstr(tag,"visible") )
                    {
                        if (strstr(tagvalue, "TRUE"))
                            new_wid->visible = true ;
                        else
                            new_wid->visible = false ;
                    }
                    if ( strstr(tag,"locked") )
                    {
                        if (strstr(tagvalue, "TRUE"))
                            new_wid->locked = true ;
                        else
                            new_wid->locked = false ;
                    }
                    if (groupid & TAG_LEGEND)
                    {
                        if ( strstr(tag,"text") )
                        {
                            delete new_wid->legend_text;
                            new_wid->legend_text = new char [strlen(tagvalue)+1] ;
                            strcpy(new_wid->legend_text, tagvalue) ;
                        }
                        if ( strstr(tag,"pos") )
                        { 
                            if (strstr(tagvalue, "PUPLACE_TOP_LEFT")) val_legend_place = PUPLACE_TOP_LEFT ; 
                            if (strstr(tagvalue, "PUPLACE_TOP_CENTERED")) val_legend_place = PUPLACE_TOP_CENTERED ;
                            if (strstr(tagvalue, "PUPLACE_TOP_RIGHT")) val_legend_place = PUPLACE_TOP_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_CENTERED_LEFT")) val_legend_place = PUPLACE_CENTERED_LEFT ;
                            if (strstr(tagvalue, "PUPLACE_CENTERED_RIGHT")) val_legend_place = PUPLACE_CENTERED_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_BOTTOM_LEFT")) val_legend_place = PUPLACE_BOTTOM_LEFT ;
                            if (strstr(tagvalue, "PUPLACE_BOTTOM_CENTERED")) val_legend_place = PUPLACE_BOTTOM_CENTERED ;
                            if (strstr(tagvalue, "PUPLACE_BOTTOM_RIGHT")) val_legend_place = PUPLACE_BOTTOM_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_CENTERED_CENTERED")) val_legend_place = PUPLACE_CENTERED_CENTERED ;
                        }
                    }
                    if (groupid & TAG_LABEL)
                    {
                        if ( strstr(tag,"text") )
                        {
                            delete new_wid->label_text;
                            new_wid->label_text = new char [strlen(tagvalue)+1] ;
                            strcpy(new_wid->label_text, tagvalue) ;
                        }
                        if ( strstr(tag,"pos") )
                        { 
                            if (strstr(tagvalue, "PUPLACE_TOP_LEFT")) val_label_place = PUPLACE_TOP_LEFT ; 
                            if (strstr(tagvalue, "PUPLACE_TOP_CENTERED")) val_label_place = PUPLACE_TOP_CENTERED ;
                            if (strstr(tagvalue, "PUPLACE_TOP_RIGHT")) val_label_place = PUPLACE_TOP_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_CENTERED_LEFT")) val_label_place = PUPLACE_CENTERED_LEFT ;
                            if (strstr(tagvalue, "PUPLACE_CENTERED_RIGHT")) val_label_place = PUPLACE_CENTERED_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_BOTTOM_LEFT")) val_label_place = PUPLACE_BOTTOM_LEFT ;
                            if (strstr(tagvalue, "PUPLACE_BOTTOM_CENTERED")) val_label_place = PUPLACE_BOTTOM_CENTERED ;
                            if (strstr(tagvalue, "PUPLACE_BOTTOM_RIGHT")) val_label_place = PUPLACE_BOTTOM_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_ABOVE_LEFT")) val_label_place = PUPLACE_ABOVE_LEFT ;
                            if (strstr(tagvalue, "PUPLACE_ABOVE_RIGHT")) val_label_place = PUPLACE_ABOVE_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_BELOW_LEFT")) val_label_place = PUPLACE_BELOW_LEFT ;
                            if (strstr(tagvalue, "PUPLACE_BELOW_RIGHT")) val_label_place = PUPLACE_BELOW_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_UPPER_LEFT")) val_label_place = PUPLACE_UPPER_LEFT ;
                            if (strstr(tagvalue, "PUPLACE_UPPER_RIGHT")) val_label_place = PUPLACE_UPPER_RIGHT ;
                            if (strstr(tagvalue, "PUPLACE_LOWER_LEFT")) val_label_place = PUPLACE_LOWER_LEFT ;
                            if (strstr(tagvalue, "PUPLACE_LOWER_RIGHT")) val_label_place = PUPLACE_LOWER_RIGHT ;
                        }
                    }
                    if (groupid & TAG_SIZE)
                    {
                        if ( strstr(tag,"width") )
                            val_object_width = atoi(tagvalue) ;
                        if ( strstr(tag,"height") )
                            val_object_height = atoi(tagvalue) ;
                    }
                    if (groupid & TAG_POSITION)
                    {
                        if ( strstr(tag,"x") )
                            wid_val_x = atoi(tagvalue) ;
                        if ( strstr(tag,"y") )
                            wid_val_y = atoi(tagvalue) ;
                    }
                    if (groupid & TAG_DATA)
                    {
                        if ( strstr(tag,"int1") )
                            intval1 = atoi (tagvalue) ;
                        if ( strstr(tag,"int2") )
                            intval2 = atoi (tagvalue) ;
                        if ( strstr(tag,"bool1") )
                        {
                            if (strstr(tagvalue, "TRUE"))
                                boolval1 = true ;
                            else
                                boolval1 = false ;
                        }
                        if ( strstr(tag,"bool2") )
                        {
                            if (strstr(tagvalue, "TRUE"))
                                boolval2 = true ;
                            else
                                boolval2 = false ;
                        }
                        if ( strstr(tag,"bool3") )
                        {
                            if (strstr(tagvalue, "TRUE"))
                                boolval3 = true ;
                            else
                                boolval3 = false ;
                        }
                        if ( strstr(tag,"float1") )
                            floatval1 = (float)atof (tagvalue) ;
                        if ( strstr(tag,"float2") )
                            floatval2 = (float)atof (tagvalue) ;
                        if ( strstr(tag,"float3") )
                            floatval3 = (float)atof (tagvalue) ;
                        if ( strstr(tag,"float4") )
                            floatval4 = (float)atof (tagvalue) ;
                        if ( strstr(tag,"float5") )
                            floatval5 = (float)atof (tagvalue) ;
                        if ( strstr(tag,"float6") )
                            floatval6 = (float)atof (tagvalue) ;

                        if ( strstr(tag,"allowed") )
                        {
                            new_wid->allowed = new char [strlen(tagvalue)+1] ;
                            strcpy(new_wid->allowed, tagvalue) ;
                        }

                        if ( strstr(tag,"item") )
                        {
                            strcat(tagvalue, "\n");
                            if (!new_wid->items)
                            {
                                new_wid->items = new char [1024] ;
                                strcpy(new_wid->items, tagvalue) ;
                            }
                            else
                                strcat(new_wid->items, tagvalue) ;
                        }
                    }
               }
            }
        }
     
        if (strstr(tag, "/savefile"))
        {
             static int mouse_x ;
             static int mouse_y ;
            setStatusWidgets( new_wid );
            glutSetWindow ( main_window ) ;
            glutSetWindowTitle ( window_name->getStringValue() ) ;
            /* Fake out the Reshape parts that determine whether to move widgets around or not. */
            mouse_x = 999 ; 
            mouse_y = 999 ;
            glutReshapeWindow ( window_size_x->getValue(), window_size_y->getValue() ) ;
            glutSetWindow ( status_window ) ;
        }

        buffer[0] = '\0';
    }
    fclose (in);
    WidgetList *wid = widgets;
    /* Check name for duplication */
     nextwid = wid;  /* Note: Yes, this is inelegant, but if we loop here, we end up making many copies of the same dialog box in the same memory space. This is a Bad Thing. */
     chk_dupname(nextwid);
    /* Done checking name for duplication */
}

static void chk_dupname_ok_cb ( puObject *ob )
{
  /* The user's new name from the dialog has already been set to the object_name*/
  /* Close dialogbox and update main window ...*/
  puDeleteObject ( dup_dialog ) ;
  dup_dialog = (puDialogBox *)NULL ;
  dup_newname = (puInput *)NULL ;
  /*but force a recheck afterwards to ensure it's an okay substitution. */
  nextwid = nextwid->next ;
  chk_dupname(nextwid);
}

static void chk_dupname ( WidgetList *new_wid )
{
    if (!new_wid)
        return;
    WidgetList *wid = widgets ;
    while ( wid )
    {
        if ( (strcmp(new_wid->object_name,wid->object_name) == 0) && (new_wid != wid) )
        {
            /* Popup a dialog telling user something's bad!  */
            dup_dialog = new puDialogBox ( 20, 20 ) ;
            new puFrame ( 0, 0, 460, 120 ) ;
            puText *text = new puText ( 80, 85 ) ;
            text->setLabel ( "ERROR: Name already used." ) ;
            text->setLabelFont(PUFONT_TIMES_ROMAN_24);
            puText *directions = new puText ( 90, 65 ) ;
            directions->setLabel ( "Please type in a new, unique name to continue." ) ;
            directions->setLabelFont(PUFONT_HELVETICA_12);
            dup_newname = new puInput (20,40,440,60) ;
            dup_newname->setValuator(new_wid->object_name);
            dup_newname->setValidData("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_012345679");
            puOneShot *ok = new puOneShot ( 200, 10, "Accept" ) ;
            ok->setCallback ( chk_dupname_ok_cb ) ;
            dup_dialog->close () ;
            dup_dialog->reveal () ;
            break ;
        }
        wid = wid->next ;
    }
}


