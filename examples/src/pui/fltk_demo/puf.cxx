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


#include "puf.h"
#include <FL/Fl.H>
#include <plib/pu.h>


int puf_translate_key(int key)
{
    const short xtab[][2] = {
	{ FL_BackSpace,    '\b' },
	{ FL_Tab,          '\t' },
	{ FL_Enter,        '\r' },
	{ FL_Escape,       0x1b },
	{ FL_Home,         PU_KEY_HOME      },
	{ FL_Left,         PU_KEY_LEFT      },
	{ FL_Up,           PU_KEY_UP        },
	{ FL_Right,        PU_KEY_RIGHT     },
	{ FL_Down,         PU_KEY_DOWN      }, 
	{ FL_Page_Up,      PU_KEY_PAGE_UP   },
	{ FL_Page_Down,    PU_KEY_PAGE_DOWN },
	{ FL_End,          PU_KEY_END       },
	{ FL_Insert,       PU_KEY_INSERT    },
	{ FL_KP + 0,       '0'  },
	{ FL_KP + 1,       '1'  },
	{ FL_KP + 2,       '2'  },
	{ FL_KP + 3,       '3'  },
	{ FL_KP + 4,       '4'  },
	{ FL_KP + 5,       '5'  },
	{ FL_KP + 6,       '6'  },
	{ FL_KP + 7,       '7'  },
	{ FL_KP + 8,       '8'  },
	{ FL_KP + 9,       '9'  },
	{ FL_KP_Enter,     '\r' },
	{ FL_F + 1,        PU_KEY_F1  },
	{ FL_F + 2,        PU_KEY_F2  },
	{ FL_F + 3,        PU_KEY_F3  },
	{ FL_F + 4,        PU_KEY_F4  },
	{ FL_F + 5,        PU_KEY_F5  },
	{ FL_F + 6,        PU_KEY_F6  },
	{ FL_F + 7,        PU_KEY_F7  },
	{ FL_F + 8,        PU_KEY_F8  },
	{ FL_F + 9,        PU_KEY_F9  },
	{ FL_F + 10,       PU_KEY_F10 },
	{ FL_F + 11,       PU_KEY_F11 },
	{ FL_F + 12,       PU_KEY_F12 },
	{ FL_Delete,       0x7f },
    };
    
    static short xmap[256] = { 0 };

    if (xmap[ FL_BackSpace & 0xff ] == 0) {
	for (unsigned i = 0; i < sizeof(xtab) / sizeof(xtab[0]); i++)
	    xmap[ xtab[i][0] & 0xff ] = xtab[i][1];
    }
    
    return key <= 0xff ? key : key >= 0xff00 ? xmap[ key & 0xff ] : 0;
}


int puf_handle_event(Fl_Gl_Window *window, int event)
{
    int n, ret = 1;

    switch (event) {

    case FL_PUSH:
    case FL_RELEASE:
	window->make_current();
	ret = puMouse(Fl::event_button() - 1, event == FL_PUSH ? PU_DOWN : PU_UP, 
		      Fl::event_x(), Fl::event_y());
	window->redraw();
	break;

    case FL_DRAG:
	window->make_current();
	ret = puMouse(Fl::event_x(), Fl::event_y());
	window->redraw();
	break;
	
    case FL_ENTER:
	window->take_focus();
	break;
	
    case FL_FOCUS:
    case FL_UNFOCUS:
	window->redraw();
	break;

    case FL_KEYDOWN:
    case FL_KEYUP:
	n = Fl::event_length();
	ret = 0;
	if (Fl::event_key() <= 0xff && n > 0) {
	    const char *s = Fl::event_text();
	    window->make_current();
	    for (int i = 0; i < n; i++)
		ret |= puKeyboard(s[i], event == FL_KEYDOWN ? PU_DOWN : PU_UP);
	} else {
	    int key = puf_translate_key(Fl::event_key());
	    if (key != 0) {
		window->make_current();
		ret = puKeyboard(key, event == FL_KEYDOWN ? PU_DOWN : PU_UP);
	    }
	}
	if (ret != 0)
	    window->redraw();
	break;

    default:
	ret = 0;
    }

    return ret;
}


int Puf_Window::handle(int event) 
{
    return puf_handle_event(this, event) || Fl_Gl_Window::handle(event);
}


void Puf_Window::draw()
{
    if (!valid())
	glViewport(0, 0, w(), h());

    glClear(GL_COLOR_BUFFER_BIT);

#if 1  // draw a border showing current focus

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w(), 0, h(), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    float c = (Fl::focus() == this ? 0.9f : 0.5f);
    glColor3f(c, c, c);

    float x0 = 0.5f, x1 = w() - 0.5f;
    float y0 = 0.5f, y1 = h() - 0.5f;
    
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();

#endif

    puDisplay();
}


/*
  Local Variables:
  mode: C++
  c-basic-offset: 4
  c-file-offsets: ((substatement-open 0) (case-label 0))
  End:
*/
