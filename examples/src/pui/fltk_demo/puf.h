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


#ifndef puf_h
#define puf_h

#include <FL/Fl_Gl_Window.H>


// Functions for passing FLTK events to PUI

int puf_translate_key(int key_code);
int puf_handle_event(Fl_Gl_Window *, int event);


// An FLTK window class for simple PUI integration

class Puf_Window : public Fl_Gl_Window
{
 protected:
    virtual int handle(int);
    virtual void draw();
 public:
    Puf_Window(int X, int Y, int W, int H, const char *L = 0)
	: Fl_Gl_Window(X, Y, W, H, L) {}
    Puf_Window(int W, int H, const char *L = 0)
	: Fl_Gl_Window(W, H, L) {}
};


#endif


/*
  Local Variables:
  mode: C++
  c-basic-offset: 4
  c-file-offsets: ((substatement-open 0) (case-label 0))
  End:
*/
