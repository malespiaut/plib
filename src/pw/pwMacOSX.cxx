
/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

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


#include "ul.h"

/* ONLY COMPILE THIS FILE FOR MACINTOSH OR MACOSX SYSTEMS */

/* PLEASE DON'T REMOVE THIS LINE AGAIN!!! */
#if defined(UL_MACINTOSH) || defined(UL_MAC_OSX)
/* YES - THAT MEANS YOU! */


/*
*
*  - the fullscreen mode is indeed a "game mode";
*  - the fullscreen mode does not change the resolution of the screen:
*		it uses the resolution (width, height and depth) in use before entering pw;
*  - it is possible to toggle between modes by calling pwCleanup()
*		and calling again pwInit;
*  - the "border" parameter in pwInit is not used;
*  - the "about" menu shows a standard alert box with the current version of plib
*		and with the url to the plib official site.
*
*-----------------------------------------
*
*  - with Mac OS 8.6 to 9.2:
*		- must be linked with the following libraries:
*			InterfaceLib, accessors.o, AppearanceLib, DrawSprocketLib
*			and OpenGL (agl is in OpenGL)
*
*		- ACTIVE_SLEEPTIME	must be defined to 0 for the fastest execution; 
*			but it will not let other app to get events.
*
*  - with Mac OS X: must be linked with the following frameworks:
*			Carbon, AGL
*
*
*  - if not using Xcode, the following compiler falgs should be set to avoid errors or warnings
*		CPPFLAGS = -fpascal-strings -funsigned-char
*/

/*  version of 2004-04-03
*   - changes from previous version:
*		- fullscreen also implemented with mac OS 8.6 and 9;
*		- key up events are now reported in mac OS 8.6 to 9.2
*			(these events are disabled by default...)
*		- multisample added for mac OS X;
*		- no auto-key repeat event by default (can be set by pwSetAutoRepeatKey(bool on_off) );
*		- better error handling and use of ulSetError;
*/

#if defined (UL_MAC_OSX)
#  define TARGET_API_MAC_CARBON 1
#  include <Carbon/Carbon.h>
#  include <AGL/agl.h>
#elif defined (UL_MACINTOSH)
//#  define TARGET_API_MAC_CARBON 0
#  undef ACCESSOR_CALLS_ARE_FUNCTIONS		// already defined by ul.h !!!
#  define ACCESSOR_CALLS_ARE_FUNCTIONS 1
#  include <Events.h>
#  include <MacWindows.h>
#  include <Dialogs.h>
#  include <ToolUtils.h>
#  include <Quickdraw.h>
#  include <Menus.h>
#  include <Gestalt.h>
#  include <Appearance.h>
#  include <Devices.h>
#  include <CodeFragments.h>
#  include <agl.h>
#  include <DrawSprocket.h>
//#else
//#  error only for Mac OS (8.6 to OS X) operating system!
#endif


#include <stdlib.h>
#include <stdio.h>
#include "pw.h"


/* Apple menu: */
#define mApple	    128
#define	iAbout	    1
/* File menu: */
#ifdef UL_MAC_OSX
#  define ACTIVE_SLEEPTIME	0
#  define INACTIVE_SLEEPTIME  10
#  define mFile		129
#  define iQuit		1
#else   // UL_MACINTOSH
#  define ACTIVE_SLEEPTIME	1		// 0 is fastest, but does not let other app to catch events.
									// set to 0 if you want your app only to have CPU time
#  define INACTIVE_SLEEPTIME  10
#  define mFile	    129
#  define iQuit	    1
#endif

static bool			pwInitialized = false ;
static int          origin [2]   = {   0,   0 } ;
static int          size   [2]   = { 640, 480 } ;
static SInt16		horScrSze, verScrSze;			// screen dimensions
static bool			full_screen;
static bool			auto_repeat_key = false ;
#if TARGET_API_MAC_CARBON
static bool			pwQuitFlag ;
#endif
static int 			modifiers ;
static int			click_modifiers ;
static SInt32 		sleepTime = ACTIVE_SLEEPTIME;

static pwResizeCB     *resizeCB = NULL ;
static pwExitCB       *exitCB   = NULL ;
static pwKeybdFunc    *kbCB     = NULL ;
static pwMouseFunc    *msCB     = NULL ;
static pwMousePosFunc *mpCB     = NULL ;

#if TARGET_API_MAC_CARBON
  static WindowPtr 	pwWindow = NULL ;
# define SETPORT(w) SetPortWindowPort(w)
# define GRAFPTR
#else
  static CWindowPtr	pwWindow = NULL ;
# define SETPORT(w) SetPort((GrafPtr)w)
# define GRAFPTR (GrafPtr)
#endif

#ifdef UL_MACINTOSH
// DrawSprocket stuff:
static DSpContextReference		dspContext;
#endif

// generic OpenGL stuff
static AGLContext   	currContext = NULL ;

static void HandleEvents(void);
static void Initialize(void);
static bool MakeWindow(int x, int y, int w, int h, char* title);
static void CreateContext(int multisample, int num_samples, bool fullscreen);
static void MakeMenu(void);
static void handleMenuEvent(long menuResult);
static void handleKeyEvent(EventRecord* eventPtr, int updown);
static void handleMouseMoveEvt(EventRecord* eventPtr);
#if TARGET_API_MAC_CARBON
static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon);
#endif
static void pwAboutBox(void);
static void defaultExitFunc ();
#ifdef UL_MACINTOSH
static bool InitDSP();
#endif


static void CtoPcpy( Str255 pstr, char *cstr );	


#if TARGET_API_MAC_CARBON
static pascal OSErr 
QuitAppleEventHandler( const AppleEvent*, AppleEvent*, long )
{
	pwQuitFlag =  true;
	return(noErr);
}
#endif

static void Initialize()
{
	OSErr	err = noErr;
	BitMap  scrBits;

#if !TARGET_API_MAC_CARBON
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	InitDialogs(nil);
	sleepTime = ACTIVE_SLEEPTIME;
	SetEventMask(everyEvent); 		// to enable key up events !!!
#else
	pwQuitFlag = false;
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false );
  // this funtion is to handle the "cmd+Q" event. This key event is not send as a keyDown event with carbon!
	if (err != noErr)
	{
		ulSetError(UL_FATAL, "Error in initialization of the system." );
	}
#endif
	InitCursor();
	exitCB = defaultExitFunc;
	// get screen size:
	GetQDGlobalsScreenBits(&scrBits);
	horScrSze = scrBits.bounds.right -  scrBits.bounds.left;
	verScrSze = scrBits.bounds.bottom -  scrBits.bounds.top;
}

// Copy a C string into a Pascal string
static void CtoPcpy(  Str255 pstr, char* cstr )
{
	int		i = 1;	
	while( (*cstr) && (i < 255) )  pstr[i++] = *cstr++;
	pstr[0] = i - 1;
}

static bool MakeWindow(int x, int y, int w, int h, char* title)
{
	Rect		wRect;
	Str255		pTitle;
	
	CtoPcpy( pTitle, title );
	SetRect(&wRect, x, y, w+x, h+y);
#if TARGET_API_MAC_CARBON
	pwWindow = NewCWindow(nil, &wRect, pTitle, true, zoomNoGrow, (WindowPtr) -1, true, 0);
#else
	if (full_screen)		// use DrawSprocket
		pwWindow = (CWindowPtr) NewCWindow (nil, &wRect, pTitle, true, plainDBox, (WindowPtr)-1, 0, 0); 
	else
		pwWindow = (CWindowPtr) NewCWindow(nil, &wRect, pTitle, true, zoomNoGrow, (WindowPtr) -1, true, 0);
#endif
	if (pwWindow == NULL)
	{
		ulSetError(UL_WARNING, "pw: can not open a window; will exit");
		return false;
	}
	SETPORT(pwWindow);
	return true;
}

static void CreateContext(int multisample, int num_samples, bool fullscreen)
{
	AGLPixelFormat aglPixFmt;
	// Choose pixel format:
	GLint attrib[10];
	GLint i = 0;
	attrib[i++] = AGL_RGBA;
	attrib[i++] = AGL_DOUBLEBUFFER;
#ifdef UL_MAC_OSX
	if (fullscreen) attrib[i++] = AGL_FULLSCREEN;
#endif
	attrib[i++] = AGL_DEPTH_SIZE;
	attrib[i++] = 32;
	//attrib[i++] = AGL_ALL_RENDERERS;
	attrib[i++] = AGL_NONE;
	
	if (multisample)
	{
#ifdef UL_MAC_OSX
		i--;
		attrib[i++] = AGL_SAMPLE_BUFFERS_ARB ; 
		attrib[i++] = 1 ;
		attrib[i++] = AGL_SAMPLES_ARB ;
		attrib[i++] = num_samples;
		attrib[i++] = AGL_NONE;
		aglPixFmt = aglChoosePixelFormat(NULL, 0, attrib);
		if (aglPixFmt == NULL)
		{
			multisample = 0;	// retry without
			ulSetError(UL_WARNING, "pw: multisample pixel format not found");
			i -= 5;
			attrib[i++] = AGL_NONE;
		}
#else
			multisample = 0;	// retry without
			ulSetError(UL_WARNING, "pw: multisample not implemented with MacOS 8 & 9"); 
#endif
	}
	if (!multisample)
	{
		aglPixFmt = aglChoosePixelFormat(NULL, 0, attrib);
	}
	if (aglPixFmt == NULL)
	{
		ulSetError(UL_FATAL, "pw: can not find a pixel format");
	}
	
	// Create an AGL context
	currContext = aglCreateContext(aglPixFmt, NULL);
	aglDestroyPixelFormat ( aglPixFmt );
	if (currContext == NULL)
	{
		ulSetError(UL_FATAL, "pw: can not create an OpenGL context\n");
	}
	// swap buffers only every k vertical retrace:
	//long k = 100;
	//aglSetInteger(currContext, AGL_SWAP_INTERVAL, &k);
}

void MakeMenu()
{
	SInt32 response;
	MenuRef menu;
	CreateNewMenu(mApple, 0, &menu);
	SetMenuTitleWithCFString( menu, CFSTR("Plib") );

	InsertMenu(menu, 0);
	InsertMenuItemTextWithCFString(menu, CFSTR("About Plib..."), 0, 0, iAbout);

#if !TARGET_API_MAC_CARBON
	AppendResMenu(menu, 'DRVR');
#endif

	// If we not running on OS X then we need to add a File:Quit command: 
    Gestalt( gestaltMenuMgrAttr, &response );
    if ( ( response & gestaltMenuMgrAquaLayoutMask ) == 0 )
	{
		menu = NewMenu (mFile, (ConstStr255Param) "File");			// new menu
		InsertMenu (menu, 0);						// add menu to end
		AppendMenu (menu, (ConstStr255Param) "Quit"); 				// add items
	}
	
	DrawMenuBar();
}

static void defaultExitFunc ()
{
  pwCleanup () ;
  exit(0);
}

void pwSetCallbacks( pwKeybdFunc    *kb, pwMouseFunc *ms,
					 pwMousePosFunc *mp, pwResizeCB *rcb,
					 pwExitCB      *ecb  )
{
	resizeCB = rcb;
	exitCB   = (ecb == NULL) ?  defaultExitFunc : ecb ;
	kbCB     = kb ;
	msCB     = ms ;
	mpCB     = mp ;
}

int pwGetModifiers()
{
	return modifiers;
}

void pwSetAutoRepeatKey( bool on_off )
{
	auto_repeat_key = on_off;
}

static void pwAboutBox()
{
	// show a dialog box with info on plib:
	SInt16 outItemHit;
	char version[32];
	Str255 Pversion;
	char* about = "for more info see <http://plib.sourceforge.net>";
	Str255 Pabout;
	sprintf(version, "PLIB v %i.%i.%i", PLIB_MAJOR_VERSION, PLIB_MINOR_VERSION, PLIB_TINY_VERSION );
	CtoPcpy( Pversion, version );
	CtoPcpy( Pabout, about );
	StandardAlert ( kAlertPlainAlert,
   					Pversion,
   					Pabout,
   					NULL,
   					&outItemHit  );
}

static void handleMenuEvent(long menuCmd)
{
#if !TARGET_API_MAC_CARBON
	Str255		DAName;
#endif
	int menuNum = HiWord(menuCmd);
	int itemNum = LoWord(menuCmd);
	switch (menuNum)
	{
		case mApple:	// apple menu:
		{
			switch (itemNum)
			{
				case iAbout: pwAboutBox(); break;
				default:
#if !TARGET_API_MAC_CARBON
					GetMenuItemText(GetMenuHandle(mApple), itemNum, DAName);
					OpenDeskAcc(DAName);
#endif
					break;
			}
			break;
		}
		case mFile:		// File menu:
		{
			switch (itemNum)
			{
				case iQuit: (*exitCB)(); break;
				default: ;
			}
		}
	}
	HiliteMenu(0);  // remove. the highlight on the selected menu.
}

static int translateKey(int ch, int key)
{
	/*
	if ( modifiers & PW_ALT )
	{
		UInt32 state = 0;
		long keyResult = KeyTranslate ( (void *) GetScriptManagerVariable (smKCHRCache),
										(UInt16) key, &state );
		ch = 0xFF & keyResult;
	}
	*/
	// special characters:
	switch (ch)
	{
		case kLeftArrowCharCode:  	ch = PW_KEY_LEFT;    break;
		case kUpArrowCharCode:  	ch = PW_KEY_UP;      break;
		case kRightArrowCharCode:  	ch = PW_KEY_RIGHT;   break;
		case kDownArrowCharCode:  	ch = PW_KEY_DOWN;    break;
		case kPageUpCharCode:  		ch = PW_KEY_PAGE_UP; break;
		case kPageDownCharCode:  	ch = PW_KEY_PAGE_DOWN; break;
		case kHomeCharCode:  		ch = PW_KEY_HOME;    break;
		case kEndCharCode:  		ch = PW_KEY_END;     break;
		case kHelpCharCode:  		ch = PW_KEY_INSERT;  break;
		case kFunctionKeyCharCode:
			// fonction keys:
			switch(key)		
			{
				case 0x7A:  ch = PW_KEY_F1;   break;
				case 0x78:  ch = PW_KEY_F2;   break;
				case 0x63:  ch = PW_KEY_F3;   break;
				case 0x76:  ch = PW_KEY_F4;   break;
				case 0x60:  ch = PW_KEY_F5;   break;
				case 0x61:  ch = PW_KEY_F6;   break;
				case 0x62:  ch = PW_KEY_F7;   break;
				case 0x64:  ch = PW_KEY_F8;   break;
				case 0x65:  ch = PW_KEY_F9;   break;
				case 0x6D:  ch = PW_KEY_F10;  break;
				case 0x67:  ch = PW_KEY_F11;  break;
				case 0x6F:  ch = PW_KEY_F12;  break;
				default: break;
			}
			break;
		default: ; 
	}
	return ch;
}

static void handleKeyEvent(EventRecord* eventPtr, int updown)
{
	GrafPtr		origPort;
	
	if ( ! kbCB ) return;
	// handle key_events:
	int ch	= eventPtr->message & charCodeMask ;		// character pressed
	int key	= (eventPtr->message & keyCodeMask) >> 8 ;	// key pressed
	ch = translateKey(ch, key);
	GetPort(&origPort);
	SETPORT(pwWindow);
	GlobalToLocal(&eventPtr->where);
	(*kbCB) ( ch, updown, eventPtr->where.h, eventPtr->where.v ) ;
	SetPort(origPort);
	return;
}


static void handleDrag(WindowPtr window, Point mouseloc)
{
	Point 		loc;
	GrafPtr		origPort;
	Rect	    dragBounds, windowBounds;

	GetRegionBounds(GetGrayRgn(), &dragBounds);

	DragWindow(window, mouseloc, &dragBounds);
		
	GetPort (&origPort);			// save Port
	SETPORT(pwWindow);

	GetWindowPortBounds(window, &windowBounds);
	loc.h = windowBounds.left;
	loc.v = windowBounds.top;

	LocalToGlobal (&loc);
	origin[0] = loc.h;				// save new origin
	origin[1] = loc.v;
	
	MoveWindow(GRAFPTR pwWindow, loc.h, loc.v, false);
	// update OpenGL Context:
	aglUpdateContext(currContext);

	//SETPORT(pwWindow);	
	SetPort (origPort);				// restore Port
}

static void handleGoAwayBox(WindowPtr window, Point mouseloc)
{
	GrafPtr		origPort;
		
	GetPort(&origPort);
	SETPORT(window);
	
	if( TrackGoAway(window, mouseloc) )
	{ 
		(*exitCB)(); 	// quit...
	}
	
	SetPort(origPort);
}

static int last_m_h = 0;
static int last_m_v = 0;

static void handleMouseMoveEvt(EventRecord* eventPtr)
{
	GrafPtr		origPort;
	
	if ( ! mpCB ) return;
	
	int m_h = eventPtr->where.h;
	int m_v = eventPtr->where.v;
	if ( (m_v != last_m_v) || (m_h != last_m_h) )
	{
		last_m_v = m_v ;    last_m_h = m_h ;		// screen coord.
		GetPort(&origPort);
		SETPORT(pwWindow);
		GlobalToLocal(&eventPtr->where);
		(*mpCB) ( eventPtr->where.h, eventPtr->where.v ) ;
		SetPort(origPort);
	}
	return;
}

static void HandleEvents()
{
  EventRecord	event;
  GrafPtr		origPort;
//  bool			testNextEvent = true;

#if TARGET_API_MAC_CARBON
  if (pwQuitFlag) (*exitCB)();		// quit!
#endif

//  while ( testNextEvent )
//  {
	WaitNextEvent(everyEvent, &event, sleepTime, nil) ;
#ifdef UL_MACINTOSH
	if (full_screen)
	{
		Boolean dspEventProcessed;
		DSpProcessEvent(&event, &dspEventProcessed);
		if (dspEventProcessed) return;
	}
#endif
    int updown = PW_UP ;

    modifiers = 0;
    if ( event.modifiers & (shiftKey | rightShiftKey) )
    	modifiers |= PW_SHIFT ;
    if ( event.modifiers & (optionKey | rightOptionKey) )
    	modifiers |= PW_ALT ;
    if ( event.modifiers & (controlKey | rightControlKey) )
    	modifiers |= PW_CTRL ;

    switch ( event.what )
    {
    	case keyDown:
    		// test for a 'command' + key (menu):
    		if(event.modifiers & cmdKey)
    		{
    			char achar	= (char) (event.message & charCodeMask);
    			handleMenuEvent( MenuKey(achar) ) ;
    			return;
    		}
    		// FALLTHROUGH (not a command)
			handleKeyEvent(&event, PW_DOWN);
			return;
    	case autoKey:
			if ( ! auto_repeat_key )
			{   // report a mouse move event, if any:
				handleMouseMoveEvt(&event);
				return;
			}
			// else report a key down event:
    		updown = PW_DOWN ;
    	case keyUp:
			handleKeyEvent(&event, updown);
			return;

    	case mouseDown:	
		{
			short	    part;
			WindowPtr	window;
		
			part = FindWindow(event.where, &window);
			switch(part)
			{
				case inMenuBar:
					handleMenuEvent( MenuSelect(event.where) );
					return;
		#if !TARGET_API_MAC_CARBON
				case inSysWindow:
					SystemClick(&event, window);
					break;
		#endif
				case inDrag:
					handleDrag(window, event.where);
					return;
				case inGoAway:
					handleGoAwayBox(window, event.where);
					return;
				case inContent:
					if(!window) return;		
					if ( GRAFPTR pwWindow != FrontWindow() )
					{
						SelectWindow (GRAFPTR pwWindow);
						return;
					}
			}
			// FALLTHROUGH (not a system click)
			updown = PW_DOWN ;
			click_modifiers = modifiers;
		}
    	case mouseUp:
    		int button;
    		switch( click_modifiers )
    		{
            	case PW_ALT :  button = PW_RIGHT_BUTTON  ; break ;
            	case PW_CTRL : button = PW_MIDDLE_BUTTON ; break ;
    			default :      button = PW_LEFT_BUTTON   ; break ;
    		}
    		GetPort(&origPort);
    		SETPORT(pwWindow);
    		GlobalToLocal(&event.where);
    		modifiers = 0;		// reset modifiers with mouse events
    		if (msCB) (*msCB)( button, updown, event.where.h, event.where.v );
    		SetPort(origPort);
    		return;

		case osEvt:
		{
			unsigned char subcode = event.message >> 24;
			if (subcode == suspendResumeMessage)   	//	Suspend/resume event	
				if (resumeFlag & event.message)		//	Resume
				{
				#ifdef UL_MACINTOSH
					if (full_screen) sleepTime = 0;
					else
				#endif
					sleepTime = ACTIVE_SLEEPTIME;
					return;
				}
				else	// suspend
					sleepTime = INACTIVE_SLEEPTIME;
			break;
		}
		case activateEvt:
			if (event.modifiers & activeFlag)
			{
				#ifdef UL_MACINTOSH
					if (full_screen) sleepTime = 0;
					else
				#endif
					sleepTime = ACTIVE_SLEEPTIME;
					return;
			}
			else sleepTime = INACTIVE_SLEEPTIME;
			break;
		
		case updateEvt:
		case nullEvent:	// likely an idle event (i.e. no events)
					// report mouse location, only if mouse has moved:
					// (there is no mouse move events on Mac!)
			handleMouseMoveEvt(&event);
			return;
		
		default : /* printf ("event: %i \n", event.what); */ break;

    }		// switch(event.what)
//  }		// while ( testNextEvent )
}


#ifdef UL_MACINTOSH
bool InitDSP()
{
	DSpContextAttributes	dspAttributes;
	DisplayIDType			displayID;
	
	if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpStartup)
	{
		ulSetError(UL_WARNING, "DrawSprocket extension not found");
		return false;
	}
	// start DrawSprocket:
	if (noErr != DSpStartup ())
	{
		ulSetError(UL_WARNING, "Unable to start DrawSprocket");
		return false;
	}
	
	// initialise to zero all fields of DSpContextAttributes:
	BlockZero( &dspAttributes, sizeof (DSpContextAttributes) );
	dspAttributes.displayWidth			= horScrSze;
	dspAttributes.displayHeight			= verScrSze;
	dspAttributes.colorNeeds			= kDSpColorNeeds_Require;
	dspAttributes.displayDepthMask		= kDSpDepthMask_16;
	dspAttributes.displayBestDepth		= 16;
	dspAttributes.backBufferDepthMask	= kDSpDepthMask_All;	// must be specified even if only fornt buffer is needed
	dspAttributes.backBufferBestDepth	= 16;
	dspAttributes.pageCount				= 1;
	//dspAttributes.contextOptions 		= 0 | kDSpContextOption_DontSyncVBL; // no page flipping and no VBL sync needed
	// look for a DSp context:
	dspContext = NULL;
	OSStatus err = DSpFindBestContext(&dspAttributes, &dspContext);
	ulSetError(UL_DEBUG, " display width = %li, height = %li,  depth = %li, mask = %lX ",
			    dspAttributes.displayWidth,
			    dspAttributes.displayHeight,
				dspAttributes.displayBestDepth,
				dspAttributes.displayDepthMask ); 
	if (err != noErr)
	{
		if (err == kDSpContextNotFoundErr)
			ulSetError(UL_WARNING, "Unable to find a DrawSprocket context");
		else
			ulSetError(UL_WARNING, "DSpFindBestContext error");
		return false;
	}
	err = DSpContext_Reserve ( dspContext, &dspAttributes );
	if (noErr != err)
	{
		ulSetError(UL_WARNING, "Unable to set the display!");
		return false;
	}
	// activate DSp context:
	//DSpContext_FadeGammaOut (NULL, NULL);		// remove for debug
	DSpContext_SetState (dspContext, kDSpContextState_Active);
	if ( !MakeWindow( 0, 0, horScrSze, verScrSze, "" ) )  return false;
	//DSpContext_FadeGammaIn (NULL, NULL);
	return true;
}
#endif

void pwInit ( int multisample, int num_samples )
{
	const char *title = "";
	pwInit ( 0,0,-1,-1, multisample, title, 0, num_samples ) ;
}


void pwInit ( int x, int y, int w, int h, int multisample,
              char *title, int border, int num_samples )
{
  if (pwInitialized)
  {
	ulSetError(UL_WARNING, "pwInit already called");
	return;
  }
  
  full_screen = ( (w<0) || (h<0) ) ? true : false ;
  Initialize();
  pwInitialized = true;
  pwWindow = NULL;
  
  // Initialize OpenGL stuff (format, context, ...):
  CreateContext(multisample, num_samples, full_screen);
  
  if (full_screen)
  {
	sleepTime = 0;
#ifdef UL_MACINTOSH
	// initialisation of fullscreen mode with DrawSprocket
	dspContext = NULL;
	if ( ! InitDSP() )
	{
		pwCleanup();	// releases GL contexts, windows, ...
		exit(4);
	}
	// attach OpenGl to the screen:
	aglSetDrawable(currContext, GetWindowPort (GRAFPTR pwWindow));
	/*
	ulSetError(UL_DEBUG, "aglSetDrawable error: %s",
					(const char*) aglErrorString(aglGetError()) );
	*/
	// activate our GL context
	aglSetCurrentContext(currContext);
#else
	aglSetCurrentContext(currContext);
	aglSetFullScreen (currContext, 0, 0, 0, 0);
#endif
	// initialise origin and size of the window (for pwGetSize)
	origin[0] = 0;
	origin[1] = 0;
	size[0] = horScrSze;
	size[1] = verScrSze;
  }
  else	// full_screen
  {
	  if ( ! MakeWindow(x,y,w,h, title) )
	  {
		  pwCleanup();	// releases GL contexts, windows, ...
		  exit(4);
	  }
	  // Attach the context to the window
  	aglSetDrawable(currContext, GetWindowPort (GRAFPTR pwWindow));
#ifdef UL_MACINTOSH
  	{	// because aglSetDrawable is slow... (not needed with OSX)
		EventRecord event;
		WaitNextEvent (everyEvent, &event, 2, NULL);
  	}
#endif
	aglSetCurrentContext(currContext);

	// create a default menu bar (with the "quit" command):
	MakeMenu();
	// initialise origin and size of the window (for pwGetSize)
	origin[0] = x;
	origin[1] = y;
	size[0] = w;
	size[1] = h;
  }
}


void pwGetSize ( int *w, int *h )
{
	*w = size[0];
	*h = size[1];
}

//special cursors:
static Cursor q_curs =		// question mark cursor
{
	{0x0380, 0x07c0, 0x0c60, 0x1830, 0x1830, 0x0030, 0x0060, 0x00c0,
	 0x0180, 0x0180, 0x0180, 0, 0, 0x0180, 0x0180, 0},	// data
	{0x0380, 0x07e0, 0x0ff0, 0x1e38, 0x1c38, 0x1c38, 0x0078, 0x00f0,
	 0x01e0, 0x01c0, 0x01c0, 0x01c0, 0, 0x01c0, 0x01c0, 0x01c0},	// mask
	{ 14, 7 } 	// hotspot
};
static Cursor c_curs =		// circle cursor
{
	{0, 0x03e0, 0x0c18, 0x1004, 0x2002, 0x2002, 0x4001, 0x4001,
	 0x4001, 0x4001, 0x4001, 0x2002, 0x2002, 0x1004, 0x0c18, 0x03e0},	// data
	{0, 0x03e0, 0x0ff8, 0x1c1c, 0x3006, 0x3006, 0x6003, 0x6003,
	 0x6003, 0x6003, 0x6003, 0x3006, 0x3006, 0x1c1c, 0x0ff8, 0x03e0},	// mask
	{ 8, 8 } 	// hotspot
};
static Cursor r_curs =		// right arrow cursor
{
	{0, 0x0008, 0x0018, 0x0038, 0x0078, 0x00f8, 0x01f8, 0x03f8,
	 0x07f8, 0x00f8, 0x00d8, 0x0188, 0x0180, 0x0300, 0x0300, 0},	 // data
	{0x000c, 0x001c, 0x003c, 0x007c, 0x00fc, 0x01fc, 0x03fc, 0x07fc,
	 0x0ffc, 0x0ffc, 0x01fc, 0x03dc, 0x03cc, 0x0780, 0x0780, 0x0700}, // mask
	{ 1, 12 } 	// hotspot
};
static Cursor x_curs =		// circle + cross cursor
{
	{0, 0x03e0, 0x0c98, 0x1084, 0x2082, 0x2082, 0x4081, 0x4081,
	 0x7fff, 0x4081, 0x4081, 0x2082, 0x2082, 0x1084, 0x0c98, 0x03e0},	// data
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0},	// mask
	{ 8, 8 } 	// hotspot
};


void pwSetCursor ( int c )
{
	UInt32 csr = 0xFFFF;
	Cursor * csrPtr;
	switch (c)
	{
		case PW_CURSOR_NONE   : HideCursor(); return;
		case PW_CURSOR_LEFT   : csr = kThemeArrowCursor; break;
		case PW_CURSOR_RIGHT  : csrPtr = &r_curs; break;
		case PW_CURSOR_AIM    : csrPtr = &x_curs; break;
		//case PW_CURSOR_CARET  : csr = kThemeIBeamCursor; break; // ???
		case PW_CURSOR_WAIT   : csr = kThemeWatchCursor; break;
		case PW_CURSOR_CROSS  : csr = kThemeCrossCursor; break;
		case PW_CURSOR_QUERY  : csrPtr = &q_curs; break;
		case PW_CURSOR_CIRCLE : csrPtr = &c_curs; break;
		case PW_CURSOR_CUSTOM :
		default : csr = kThemeArrowCursor;
				 ulSetError(UL_WARNING, "Unknown or unavailable cursor type\n");		
	}
	if (csr==0xFFFF)  SetCursor(csrPtr);
	else  SetThemeCursor(csr);
	ShowCursor();
}


void pwSetSize ( int w, int h )
{
	if (!pwInitialized)
	{
		ulSetError(UL_WARNING, "call to pwSetSize without call to pwInit");
		return;
	}
	if ( full_screen ) 
	{
		ulSetError(UL_WARNING, "call to pwSetSize in full screen mode");
		return;
	}
	if ( ( w == size[0] ) && ( h == size[1] ) ) return;
	
	GrafPtr		origPort;
	
	if (w<50) w = 50;
	if (w>horScrSze) w = horScrSze;
	if (h<50) h = 50;
	if (h>verScrSze) h = verScrSze;
	
	GetPort(&origPort);
	
	size[0] = w;		// save size
	size[1] = h;
	
	SizeWindow(GRAFPTR pwWindow, w, h, false);
	// update OpenGL Context:
	aglUpdateContext(currContext);
	
	if ( resizeCB != NULL )  (*resizeCB) ( w, h ) ;
	
	SetPort(origPort);
}


void pwSetOrigin ( int x, int y )
{
	if (!pwInitialized)
	{
		ulSetError(UL_WARNING, "call to pwSetOrigin without call to pwInit");
		return;
	}
	if ( full_screen ) 
	{
		ulSetError(UL_WARNING, "call to pwSetOrigin in full screen mode");
		return;
	}
	if ( ( x == origin[0] ) && ( y == origin[1] ) ) return;
	
	if ( (x + size[0]) < 10)   x = 10 - size[0];
	if ( x > (horScrSze-10) )  x = horScrSze-10;
	if ( (y + size[1]) < 60)   y = 60 - size[1];
	if ( y > (verScrSze-10) )  y = verScrSze-10;
	
	MoveWindow(GRAFPTR pwWindow, x, y, false);
	// update OpenGL Context:
	aglUpdateContext(currContext);
	
	origin[0] = x;				// save new origin
	origin[1] = y;
}


void pwSetSizeOrigin ( int x, int y, int w, int h )
{
	if (!pwInitialized)
	{
		ulSetError(UL_WARNING, "call to pwSetSizeOrigin without call to pwInit");
		return;
	}
	if ( full_screen ) 
	{
		ulSetError(UL_WARNING, "call to pwSetSizeOrigin in full screen mode");
		return;
	}
	
	pwSetOrigin( x, y );
	pwSetSize( w, h );
}


void pwSwapBuffers ()
{
	if (!pwInitialized)
	{
		ulSetError(UL_WARNING, "call to pwSwapBuffers without call to pwInit");
		return;
	}
	// glFlush () ;
	aglSwapBuffers ( currContext ) ;
	HandleEvents () ;
}


void pwCleanup ()
{
	if (!pwInitialized)
	{
		ulSetError(UL_WARNING, "call to pwCleanup without call to pwInit");
		return;
	}
	
	if (currContext != NULL)
	{
		aglSetDrawable (currContext, NULL);
		aglSetCurrentContext (NULL);
		aglDestroyContext ( currContext );
	}
	if (pwWindow != NULL)   DisposeWindow(GRAFPTR pwWindow);
	
#ifdef UL_MACINTOSH
	if (full_screen)
	{
		if (dspContext != NULL)
		{
			//DSpContext_FadeGammaOut (NULL, NULL);	// remove for debug
			DSpContext_SetState (dspContext, kDSpContextState_Inactive);
			//DSpContext_FadeGammaIn (NULL, NULL);	

			DSpContext_Release (dspContext);
		}
			
		DSpShutdown ();
	}
#endif

	pwInitialized = false;
}

#endif
