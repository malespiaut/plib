/*
     P-GUIDE - PUI-based Graphical User Interface Designer
     Copyright (C) 2002  John F. Fay

     This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License as
     published by the Free Software Foundation; either version 2 of
     the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/


#include <stdio.h>

#include <plib/pu.h>

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

#define CALLBACK_UP         0x00000001
#define CALLBACK_ACTIVE     0x00000002
#define CALLBACK_DOWN       0x00000004

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


extern float main_window_color_r, main_window_color_g,
             main_window_color_b, main_window_color_a ;

// From the Status Window

extern int status_window ;
extern int main_window ;
extern puFileSelector *file_selector ;

// Now our turn

void saveProject ( puObject *ob ) {
    char* filename ;
      file_selector -> getValue ( &filename ) ;
    if (filename[0] == '\0')
    {
        puDeleteObject ( file_selector ) ;
        file_selector = (puFileSelector *)NULL ;
        glutHideWindow () ;
        glutSetWindow ( status_window ) ;
        return ;
    }        

    /* If they didn't give an extension, then tack ".xml" onto the end. */
    if(!strstr(filename, "."))
        sprintf(filename, "%s.xml", filename);

    FILE *out = fopen ( filename, "wt" ) ;
    if ( !out )
     printf ( "ERROR opening file <%s> for writing\n", filename ) ;

    puDeleteObject ( file_selector ) ;
    file_selector = (puFileSelector *)NULL ;
    glutHideWindow () ;
    glutSetWindow ( status_window ) ;

    if ( !out ) return ;

  // Start writing code:
    char projectname [] = "Test Project";

    fprintf ( out, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n<!DOCTYPE P-Guide>\n<savefile>\n\n" ) ;
    fprintf ( out, "<version>VERSION</version>\n") ;
    fprintf ( out, "<project>%s</project>\n", projectname) ;
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
    char *place_name [] = { "PUPLACE_TOP_LEFT", "PUPLACE_TOP_CENTERED", "PUPLACE_TOP_RIGHT",
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
        char *hidden ;
        obj->getPosition ( &x, &y ) ;
        obj->getSize ( &w, &h ) ;

        fprintf ( out, "\t<widget>\n\t\t<name>%s</name>\n", wid->object_name) ;
        if (wid->legend_text != NULL)
            fprintf ( out, "\t\t<legend>\n\t\t\t<text>%s</text>\n\t\t\t<pos>%s</pos>\n\t\t</legend>\n", wid->legend_text, place_name [ obj->getLegendPlace () ] ) ;
        else
            fprintf ( out, "\t\t<legend>\n\t\t\t<text></text>\n\t\t\t<pos></pos>\n\t\t</legend>\n") ;

        if (wid->label_text != NULL)
            fprintf ( out, "\t\t<label>\n\t\t\t<text>%s</text>\n\t\t\t<pos>%s</pos>\n\t\t</label>\n", wid->label_text, place_name [ obj->getLabelPlace () ] ) ;
        else
            fprintf ( out, "\t\t<label>\n\t\t\t<text></text>\n\t\t\t<pos></pos>\n\t\t</label>\n") ;

        fprintf ( out, "\t\t<type>%s</type>\n", wid->object_type_name) ;

        if ( !wid->visible ) 
            hidden="FALSE";
        else
            hidden="TRUE";

        if ( wid->callbacks & CALLBACK_UP )  // Up-callback defined
          fprintf ( out, "\t\t<callbacks>Up</callbacks>\n") ;
        if ( wid->callbacks & CALLBACK_ACTIVE )  // Active callback defined
          fprintf ( out, "\t\t<callbacks>Active</callbacks>\n") ;
        if ( wid->callbacks & CALLBACK_DOWN )  // Down-callback defined
          fprintf ( out, "\t\t<callbacks>Down</callbacks>\n") ;
        if ( wid->callbacks == 0 )
          fprintf ( out, "\t\t<callbacks></callbacks>\n") ;

        //if ( !wid->layer )
            //fprintf ( out, "\t\t<layer>0</layer>\n") ;
        //else
        fprintf ( out, "\t\t<layer>%d</layer>\n", wid->layer) ;

        fprintf ( out, "\t\t<visible>%s</visible>\n", hidden) ;
        fprintf ( out, "\t\t<size>\n\t\t\t<width>%d</width>\n\t\t\t<height>%d</height>\n\t\t</size>\n", w, h) ;
        fprintf ( out, "\t\t<position>\n\t\t\t<x>%d</x>\n\t\t\t<y>%d</y>\n\t\t</position>\n", x, y) ;
        fprintf ( out, "\t</widget>\n") ;
        wid = wid->next ;
    }

    fprintf ( out, "</objects>\n</savefile>\n") ; /* XML output done */
    fclose (out) ;
}

void gotoNextLine( char* buffer )
{
    while (buffer != "\n")
        buffer++ ;
}

void loadProject ( puObject *ob ) {
    extern puInput *object_size_x ;
    extern puInput *object_size_y ;
    extern puInput *object_position_x ;
    extern puInput *object_position_y ;

    extern puInput *object_label ;
    extern puComboBox *object_vert_label_place ;
    extern puComboBox *object_horz_label_place ;
    extern puInput *object_legend ;
    extern puComboBox *object_vert_legend_place ;
    extern puComboBox *object_horz_legend_place ;

    extern puInput *object_name ;
    extern puButtonBox *object_callbacks ;

    extern puButtonBox *object_visible ;
    extern puSpinBox *object_layer ;

    extern puInput *window_name ;
    extern puInput *window_size_x ;
    extern puInput *window_size_y ;
    extern puInput *window_position_x ;
    extern puInput *window_position_y ;

    extern puSpinBox *window_color_r ;
    extern puSpinBox *window_color_g ;
    extern puSpinBox *window_color_b ;
    extern puSpinBox *window_color_a ;

    char* filename ;
    file_selector -> getValue ( &filename ) ;
    if (filename[0] == '\0')
    {
        puDeleteObject ( file_selector ) ;
        file_selector = (puFileSelector *)NULL ;
        glutHideWindow () ;
        glutSetWindow ( status_window ) ;
        return ;
    }        

    FILE *in = fopen ( filename, "rt" ) ;
    if ( !in )
        printf ( "ERROR opening file <%s> for reading\n", filename ) ;

    puDeleteObject ( file_selector ) ;
    file_selector = (puFileSelector *)NULL ;
    glutHideWindow () ;
    glutSetWindow ( status_window ) ;
    
    if ( !in ) return ;

    char rawbuffer[2048];
    // Verify file is XML

    float tver = 0.0f ;
    fgets( rawbuffer, sizeof(rawbuffer), in) ;
    sscanf(rawbuffer,"<?xml version=\"%f\" encoding=\"*%s\" ?>", &tver) ;
    if (!tver) {
        printf ("File does not contain proper XML headings.\n");
        return ;
    }

    // Verify file is P-Guide
    char tag[PUSTRING_MAX] ;
    char tagvalue[PUSTRING_MAX] ;
    fgets( rawbuffer, sizeof(rawbuffer), in) ;

    if (!sscanf(rawbuffer,"<!DOCTYPE %s>", &tag)) {
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
    char onscreen_legend_text[PUSTRING_MAX] = "" ; 

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
    
        sscanf(buffer,">%[a-zA-Z_!@#$^&*0123456789. ]</", tagvalue) ; /* Get the tag's value. */

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

        /* Now that the tag's classifcation has been determined, rip the data */
        /* viciously from its' hold and throw it onto the memory stack.       */
 
        if (groupid & TAG_SAVEFILE )
        {
            /* NULLCHECK! */
            if (tagvalue[0] == '\0') { sprintf(tagvalue, " "); }
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
                   if (new_wid->label_text[0] != '\0')
                   {
                        new_obj->setLabel ( new_wid->label_text ) ;
                        new_obj->setLabelPlace ( val_label_place ) ; /**/
                   }
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
                        if (tmp_wid_num > widget_number) widget_number = tmp_wid_num ;
                    }
                    if ( strstr(tag,"type") )
                    {
                        new_wid->object_type = 0 ;
                        new_wid->object_type_name = new char [strlen(tagvalue)+1] ;
                        strcpy(new_wid->object_type_name, tagvalue) ;
                        /*new_wid->object_type*/

                        if ( strstr(new_wid->object_type_name, "puSelectBox"))       new_wid->object_type |= PUCLASS_SELECTBOX   ;
                        if ( strstr(new_wid->object_type_name, "puComboBox"))        new_wid->object_type |= PUCLASS_COMBOBOX    ;  
                        if ( strstr(new_wid->object_type_name, "puLargeInput"))      new_wid->object_type |= PUCLASS_LARGEINPUT  ;
                        if ( strstr(new_wid->object_type_name, "puVerticalMenu"))    new_wid->object_type |= PUCLASS_VERTMENU    ;
                        if ( strstr(new_wid->object_type_name, "puTriSlider"))       new_wid->object_type |= PUCLASS_TRISLIDER   ;
                        if ( strstr(new_wid->object_type_name, "puBiSlider"))        new_wid->object_type |= PUCLASS_BISLIDER    ;
                        if ( strstr(new_wid->object_type_name, "puFileSelector"))    new_wid->object_type |= PUCLASS_FILESELECTOR;
                        if ( strstr(new_wid->object_type_name, "puDial"))            new_wid->object_type |= PUCLASS_DIAL        ;
                        if ( strstr(new_wid->object_type_name, "puListBox"))         new_wid->object_type |= PUCLASS_LISTBOX     ;
                        if ( strstr(new_wid->object_type_name, "puArrowButton"))     new_wid->object_type |= PUCLASS_ARROW       ;
                        if ( strstr(new_wid->object_type_name, "puDialogBox"))       new_wid->object_type |= PUCLASS_DIALOGBOX   ; /**/
                        if ( strstr(new_wid->object_type_name, "puSlider"))          new_wid->object_type |= PUCLASS_SLIDER      ;
                        if ( strstr(new_wid->object_type_name, "puButtonBox"))       new_wid->object_type |= PUCLASS_BUTTONBOX   ;
                        if ( strstr(new_wid->object_type_name, "puInput"))           new_wid->object_type |= PUCLASS_INPUT       ;
                        if ( strstr(new_wid->object_type_name, "puMenuBar"))         new_wid->object_type |= PUCLASS_MENUBAR     ;
                        if ( strstr(new_wid->object_type_name, "puPopupMenu"))       new_wid->object_type |= PUCLASS_POPUPMENU   ; /**/
                        if ( strstr(new_wid->object_type_name, "puPopup"))           new_wid->object_type |= PUCLASS_POPUP       ; /**/
                        if ( strstr(new_wid->object_type_name, "puOneShot"))         new_wid->object_type |= PUCLASS_ONESHOT     ;
                        if ( strstr(new_wid->object_type_name, "puButton"))          new_wid->object_type |= PUCLASS_BUTTON      ;
                        if ( strstr(new_wid->object_type_name, "puText"))            new_wid->object_type |= PUCLASS_TEXT        ;
                        if ( strstr(new_wid->object_type_name, "puFrame"))           new_wid->object_type |= PUCLASS_FRAME       ;
                        if ( strstr(new_wid->object_type_name, "puSpinBox"))         new_wid->object_type |= PUCLASS_SPINBOX     ; /**/
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
                    if (groupid & TAG_LEGEND)
                    {
                        if ( strstr(tag,"text") )
                        {
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
               }
            }
        }
     
        if (strstr(tag, "/savefile"))
        {
             static int mouse_x ;
             static int mouse_y ;
            glutSetWindow ( main_window ) ;
            glutSetWindowTitle ( window_name->getStringValue() ) ;
            mouse_x = 999 ; 
            mouse_y = 999 ;
            glutReshapeWindow ( window_size_x->getValue(), window_size_y->getValue() ) ;
            glutSetWindow ( status_window ) ;
        }

        buffer[0] = '\0';
    }
    fclose (in);
}

