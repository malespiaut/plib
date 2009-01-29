/*
     PUI Auxiliary Widget Library
     Derived from PLIB, the Portable Game Library by Steve Baker.
     Copyright (C) 1998,2002,2004  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "puAuxLocal.h"

UL_RTTI_DEF1(puaList,puGroup)
// puAuxList.cxx - implementation of a scrolling list box.

#include "puAux.h"


/**
 * Static function: handle slider movements.
 */
static void
handle_slider (puObject * slider)
{
    puListBox * list_box = (puListBox *)slider->getUserData();

    int total = list_box->getNumItems();
    int visible = list_box->getNumVisible();
    // negative numbers are OK -- setTopItem() clamps anyway
    int index = int((total - visible) * (1.0 - slider->getFloatValue()));
    list_box->setTopItem(index);
}


/**
 * Static function: handle list entry selection.
 */
static void
handle_list_entry (puObject * listbox)
{
    puaList * box = (puaList *)listbox->getUserData();
    box->invokeCallback();
}


/**
 * Static function: handle arrow clicks.
 */
static void
handle_arrow (puObject * arrow)
{
    puSlider * slider = (puSlider *)arrow->getUserData();
    puListBox * list_box = (puListBox *)slider->getUserData();
    puaList * list = (puaList *)list_box->getUserData();

    int step;
    switch (((puArrowButton *)arrow)->getArrowType()) {
    case PUARROW_DOWN:
        step = 1;
        break;
    case PUARROW_UP:
        step = -1;
        break;
    default:
        step = 0;
        break;
    }

    int index = list->getTopItem();
    list->setTopItem(index + step);
    slider->setValue(1.0f - float(list->getTopItem()) / (list->getNumItems() - list->getNumVisible()));
}

/* Create an empty list box. */
puaList::puaList (int x, int y, int w, int h, int sl_width) :
    puGroup(x, y),
    _sw(sl_width)
{
    type |= PUCLASS_LIST;
    init(w, h, 1);
}

/* Create a filled list box. */
puaList::puaList (int x, int y, int w, int h, char ** contents, int sl_width) :
    puGroup(x, y),
    _sw(sl_width)
{
    type |= PUCLASS_LIST;
    init(w, h, 1);
    newList(contents);
}

/* Create an empty list box. */
puaList::puaList (int x, int y, int w, int h, short transparent, int sl_width) :
    puGroup(x, y),
    _sw(sl_width)
{
    type |= PUCLASS_LIST;
    init(w, h, transparent);
}

/* Create a filled list box. */
puaList::puaList (int x, int y, int w, int h, short transparent,
                  char ** contents, int sl_width) :
    puGroup(x, y),
    _sw(sl_width)
{
    type |= PUCLASS_LIST;
    init(w, h, transparent);
    newList(contents);
}

puaList::~puaList ()
{
}

void
puaList::newList (char ** contents)
{
    _list_box->newList(contents);
    _contents = contents;

    // new size calculation to consider slider visibility
    setSize(_width, _height);
}

void
puaList::setTopItem (int item_index)
{
    _list_box->setTopItem(item_index);
    item_index = _list_box->getTopItem();
    // read clamped value back in, and only set slider if it doesn't match the new
    // index to avoid jumps
    int slider_index = int((1.0f - _slider->getFloatValue()) * (getNumItems() - getNumVisible()));
    if (slider_index != item_index)
        _slider->setValue(1.0f - float(getTopItem()) / (getNumItems() - getNumVisible()));
}

/*
 *  Return a pointer to a character string with the
 *  contents of the selected entry.
 */
char *
puaList::getStringValue ()
{
  int currElem = _list_box->getIntegerValue();
  if (currElem < 0)
  {
    return NULL;
  }
  else
  {
    return _contents[currElem];
  }
}

/*
 *  Set a pointer to a character string with the
 *  contents of the selected entry
 */
void
puaList::getValue(char **ps)
{
  int currElem = _list_box->getIntegerValue();
  if (currElem < 0)
  {
    *ps = NULL;
  }
  else
  {
    *ps = _contents[currElem];
  }
}


/*
 *  Set the integer pointed to by "i" to 
 *  the index of the currently selected entry.
 */
void
puaList::getValue(int *i)
{
  *i = _list_box->getIntegerValue();
}

/* Return the index of the currently selected entry. */
int
puaList::getIntegerValue()
{
  return _list_box->getIntegerValue();
}

void
puaList::setColourScheme (float r, float g, float b, float a)
{
    puObject::setColourScheme(r, g, b, a);
    _list_box->setColourScheme(r, g, b, a);
}

void
puaList::setColour (int which, float r, float g, float b, float a)
{
    puObject::setColour(which, r, g, b, a);
    _list_box->setColour(which, r, g, b, a);
}

void
puaList::setSize (int w, int h)
{
    _width = w;
    _height = h;
    puObject::setSize(w, h);
    if (_frame)
        _frame->setSize(w, h);

    int total = getNumItems();
    int visible = getNumVisible();

    if (total > visible)
    {
        if (!_slider->isVisible())
        {
            _slider->setValue(1.0f);
            _slider->reveal();
            _up_arrow->reveal();
            _down_arrow->reveal();
        }
        _list_box->setSize(w-_sw, h);

        _slider->setPosition(w-_sw, _sw);
        _slider->setSize(_sw, h-2*_sw);
        _slider->setSliderFraction(float(visible) / total);
        _slider->setPageStepSize(float(visible) / (total - visible));

        _down_arrow->setPosition(w-_sw, 0);
        _up_arrow->setPosition(w-_sw, h-_sw);

    }
    else
    {
        if (_slider->isVisible())
        {
            _slider->hide();
            _up_arrow->hide();
            _down_arrow->hide();
        }
        _list_box->setSize(w, h);
    }
}

void
puaList::init (int w, int h, short transparent)
{
  if ( transparent )
    _frame = NULL ;
  else
    _frame = new puFrame(0, 0, w, h);

  _list_box = new puListBox(0, 0, w-_sw, h);
  _list_box->setStyle(-PUSTYLE_SMALL_SHADED);
  _list_box->setUserData(this);
  _list_box->setCallback(handle_list_entry);
  _list_box->setValue(0);

  _slider = new puSlider(w-_sw, _sw, h-2*_sw, true, _sw);
  _slider->setValue(1.0f);
  _slider->setUserData(_list_box);
  _slider->setCallback(handle_slider);
  _slider->setCBMode(PUSLIDER_ALWAYS);

  _down_arrow = new puArrowButton(w-_sw, 0, w, _sw, PUARROW_DOWN) ;
  _down_arrow->setUserData(_slider);
  _down_arrow->setCallback(handle_arrow);

  _up_arrow = new puArrowButton(w-_sw, h-_sw, w, h, PUARROW_UP);
  _up_arrow->setUserData(_slider);
  _up_arrow->setCallback(handle_arrow);

  setSize(w, h);
  close();
}

// end of puAuxList.cxx
