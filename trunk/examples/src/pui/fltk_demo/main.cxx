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
#include "gui.h"
#include <plib/pu.h>


static Puf_Window *window1;
static Puf_Window *window2;

static puButton *button1;
static puButton *button2;

static puInput *input1;
static puInput *input2;


static void hello_cb(puObject *o)
{
    puInput *i = (o == button1 ? input1 : input2);   
    char *s = 0;
    i->getValue(&s);
    fprintf(stderr, "[Window %d] Hello: %s\n", o == button1 ? 1 : 2, s);
}


static void toggle_cb(puObject *o)
{
    if (main_window->visible())
	main_window->hide();
    else
	main_window->show();
}


static void quit_cb(puObject *o)
{
    main_window->hide();
    window1->hide();
}


static void init_pui()
{
    window1->make_current();

    puInit();

    new puGroup(0, 0);
    puButton *b = new puOneShot(50, 50, 200, 80);
    b->setLegend("Show / Hide");
    b->setCallback(toggle_cb);
    button1 = new puOneShot(50, 120, 200, 150);
    button1->setLegend("Say Hello");
    button1->setCallback(hello_cb);
    input1 = new puInput(50, 170, 200, 200);
    puMenuBar *menu = new puMenuBar();
    char *menu_text[] = { "Quit", "----", "----", NULL };
    puCallback menu_cb[] = { quit_cb, NULL, NULL };
    char *dummy_text[] = { "----", NULL };
    puCallback dummy_cb[] = { NULL };
    menu->add_submenu("File", menu_text, menu_cb);
    menu->add_submenu("Edit", dummy_text, dummy_cb);
    menu->add_submenu("View", dummy_text, dummy_cb);
    menu->close();
    puPopGroup();

    window2->make_current();

    new puInterface(0, 0);
    new puGroup(0, 0);
    button2 = new puOneShot(50, 50, 200, 80);
    button2->setLegend("Say Hello");
    button2->setCallback(hello_cb);
    input2 = new puInput(50, 100, 200, 130);
    puPopGroup();
}


static void toggle_fl(Fl_Widget *widget, void *data)
{
    if (window1->visible())
	window1->hide();
    else
	window1->show();
}


// This is for debugging, to see that redraw occurs when it should.
static void refresh(void *data)
{
    window1->redraw();
    window2->redraw();
    Fl::repeat_timeout(2.0, refresh);
}


int main(int argc, char **argv)
{
    window1 = new Puf_Window(250, 270, "OpenGL Window");
    window1->show();

    make_window()->show();
    window2 = gl_area;

    init_pui();

    toggle_other->callback(toggle_fl);
    Fl::add_timeout(3.0, refresh);

    return Fl::run();
}


/*
  Local Variables:
  mode: C++
  c-basic-offset: 4
  c-file-offsets: ((substatement-open 0) (case-label 0))
  End:
*/
