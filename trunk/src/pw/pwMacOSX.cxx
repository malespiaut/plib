

/*
 *  - with Mac OS 8.6 to 9.2:
 *		- must be linked with the following libraries:
 *			InterfaceLib, accessors.o, AppearanceLib and OpenGL (it contains agl)
 *
 *		- ACTIVE_SLEEPTIME	must be defined to 0 for the fastest execution; 
 *			but it will not let other app to get events.
 *
 *  - with Mac OS X: must be linked with the following frameworks:
 *			Carbon, AGL
 *
 */

#if defined(UL_MACINTOSH) || defined(UL_MAC_OSX)

#ifdef UL_MAC_OSX
#  define TARGET_API_MAC_CARBON 1
#  include <Carbon/Carbon.h>
#  include <AGL/agl.h>
#elifdef UL_MACINTOSH 
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
#  include <agl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "ul.h"
#include "pw.h"


/* Apple menu: */
#define mApple	    128
#define	iAbout	    1
/* File menu: */
#  define mFile	    129
#  define iQuit	    1

#ifdef UL_MAC_OSX
#  define ACTIVE_SLEEPTIME	0
#  define INACTIVE_SLEEPTIME  10
#else   // UL_MACINTOSH
#  define ACTIVE_SLEEPTIME	1		// 0 is fastest, but does not let other app to catch events.
									// set to 0 if you want your app only to have CPU time
#  define INACTIVE_SLEEPTIME  10
#endif

static bool			pwInitialized = false ;
static int          origin [2]   = {   0,   0 } ;
static int          size   [2]   = { 640, 480 } ;
static SInt16		horScrSze, verScrSze;			// screen dimensions
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
    // generic OpenGL stuff
static AGLContext   currContext = NULL ;

static void Initialize(void);
static void MakeWindow(int x, int y, int w, int h, char* title);
static void CreateContext(int multisample, int num_samples, bool fullscreen);
static void MakeMenu(void);
void pwHandleEvents(void);
//static void DoEvent(EventRecord *event);
static void handleMenuEvent(long menuResult);
//static void HandleWindowUpdate(WindowPtr window);
#if TARGET_API_MAC_CARBON
static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon);
#endif
static void pwAboutBox(void);
static void defaultExitFunc ();


static void CtoPcpy( Str255 pstr, char *cstr );	// déjà dans plib ! (?)
//extern void logOut(const char*, ...);


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
	// MaxApplZone();		// only for 68k code!

	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	// TEInit();
	InitDialogs(nil);
	//qd.randSeed =  TickCount();
	sleepTime = ACTIVE_SLEEPTIME;
#endif
  InitCursor();
  exitCB = defaultExitFunc;
#if TARGET_API_MAC_CARBON
  pwQuitFlag = false;
  err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false );
  // this funtion is to handle the "cmd+Q" event. This key event is not send as a keyDown event with carbon!
#endif
	// get screen size:
	GetQDGlobalsScreenBits(&scrBits);
	horScrSze = scrBits.bounds.right -  scrBits.bounds.left;
	verScrSze = scrBits.bounds.bottom -  scrBits.bounds.top;

	if (err != noErr)
	{
		ulSetError(UL_FATAL, "Error in initialization of the system." );
		exit(3); //ExitToShell(); 
	}
}

// Copy a C string into a Pascal string
static void CtoPcpy(  Str255 pstr, char* cstr )
{
	int		i = 1;	
	while( (*cstr) && (i < 255) )  pstr[i++] = *cstr++;
	pstr[0] = i - 1;
}

static void MakeWindow(int x, int y, int w, int h, char* title)
{
	Rect		wRect;
	Str255		pTitle;
	
	CtoPcpy( pTitle, title );
	SetRect(&wRect, x, y, w+x, h+y);
#if TARGET_API_MAC_CARBON
	pwWindow = NewCWindow(nil, &wRect, pTitle, true, zoomNoGrow, (WindowPtr) -1, true, 0);
#else
	pwWindow = (CWindowPtr) NewCWindow(nil, &wRect, pTitle, true, zoomNoGrow, (WindowPtr) -1, true, 0);
#endif
	if (pwWindow == nil)
	{
		ulSetError(UL_FATAL, "pw: can not open a window\n");
		exit(3);
	}
	SETPORT(pwWindow);
}

static void CreateContext(int multisample, int num_samples, bool fullscreen)
{
	AGLPixelFormat aglPixFmt;
	// Choose pixel format:
	GLint attrib[10];
	GLint i = 0;
	attrib[i++] = AGL_RGBA;
	attrib[i++] = AGL_DOUBLEBUFFER;
#if TARGET_API_MAC_CARBON
	if (fullscreen) attrib[i++] = AGL_FULLSCREEN;
#endif
	//attrib[i++] = AGL_DEPTH_SIZE;
	//attrib[i++] = 24;
	//attrib[i++] = AGL_ALL_RENDERERS;
	attrib[i++] = AGL_NONE;
	attrib[i++] = AGL_NONE;
	attrib[i++] = AGL_NONE;
	
	if (multisample)
	{
		// attrib[4] = ...
		// aglPixFmt = aglChoosePixelFormat(NULL, 0, attrib);
		// if (aglPixFmt == NULL)
		//{
			multisample = 0;	// retry without
			ulSetError(UL_WARNING, "pw: multisample not implemented with MacOS 8 & 9\n"); 
		//}
	}
	if (!multisample)
	{
		// without multisample:
		//attrib[4] = AGL_NONE ;
		aglPixFmt = aglChoosePixelFormat(NULL, 0, attrib);
	}
	if (aglPixFmt == NULL)
	{
		ulSetError(UL_FATAL, "pw: can not find a pixel format\n");
		//printf("pw: pixel format  -  Exit\n");
		//logOut( (const char*) aglErrorString(aglGetError()) );
  		//logOut(" - can not find a pixel format!\n");
		exit(2);
	}
	
	// Create an AGL context
	currContext = aglCreateContext(aglPixFmt, NULL);
	aglDestroyPixelFormat ( aglPixFmt );
	if (currContext == NULL)
	{
		ulSetError(UL_FATAL, "pw: can not create an OpenGL context\n");
  		//logOut("can not create context\n");
		exit(1);
	}
	// swap buffers only during vertical retrace:
	//long k = 100;
	//aglSetInteger(currContext, AGL_SWAP_INTERVAL, &k);
}

void MakeMenu()
{
	OSErr err;
	long response;
	MenuHandle menu = NewMenu(mApple, "\p\024");
	InsertMenu(menu, 0);

	InsertMenuItem(menu, "\pAbout Plib...", 0);

#if !TARGET_API_MAC_CARBON
	AppendResMenu(menu, 'DRVR');
#endif

	// If we not running on OS X then we need to add a File:Quit command: 
	err = Gestalt(gestaltSystemVersion, &response );

	if (err != noErr || response < 0x00001000)
	{
		menu = NewMenu (mFile, "\pFile");			// new menu
		InsertMenu (menu, 0);						// add menu to end
		AppendMenu (menu, "\pQuit/Q"); 				// add items
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

static void pwAboutBox()
{
	// show a dialog box with info on plib:
	SInt16 outItemHit;
	char version[32];
	Str255 Pversion;
	sprintf(version, "PLIB v %i.%i.%i", PLIB_MAJOR_VERSION, PLIB_MINOR_VERSION, PLIB_TINY_VERSION );
	CtoPcpy( Pversion, version );
	StandardAlert ( kAlertPlainAlert,
   					"\pPLIB v 1.8.1",
   					"\pfor more infos see <http://plib.sourceforge.net>",
   					NULL,
   					&outItemHit  );

	//printf("pwAboutBox appelé\n");
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
			//logOut("key = %X \n", key);
			switch(key)		// do not work !!!
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

/*
static void handleGrow(WindowPtr window, Point mouseloc)
{
	Rect	growBounds;
	long	newSize;
	GrafPtr		origPort;
			
	GetPort(&origPort);
	
	SETPORT(window);
	GetRegionBounds(GetGrayRgn(), &growBounds);

	growBounds.right -= growBounds.left;
	growBounds.bottom -= growBounds.top;
	growBounds.left = 20;
	growBounds.top = 20;
	
	newSize = GrowWindow(window, mouseloc, &growBounds);
	
	if(newSize)
	{
		size[0] = LoWord(newSize);		// save size
		size[1] = HiWord(newSize);
		
		SizeWindow(window, size[1], size[2], false);
		if ( resizeCB != NULL )  (*resizeCB) ( size[0], size[1] ) ;
	}
	
	SetPort(origPort);
}

static void handleZoom(WindowPtr window, Point mouseloc, short zoom)
{
	GrafPtr		origPort;
	Rect 		windowBounds;
	
	GetPort(&origPort);

	SETPORT(window);
	
	if( TrackBox(window, mouseloc, zoom) )
	{
		//DoZoomWindow (window, zoom);
		GetWindowPortBounds(window, &windowBounds); 
		EraseRect(&windowBounds);	// erase to avoid flicker
		ZoomWindow (window, zoom, false);
		
		size[0] = windowBounds.right - windowBounds.left;
		size[1] = windowBounds.top - windowBounds.bottom;
		if ( resizeCB != NULL )  (*resizeCB) ( size[0], size[1] ) ;
	}
	
	SetPort(origPort);
}
*/
static int last_m_h = 0;
static int last_m_v = 0;

void pwHandleEvents()
{
  EventRecord	event;
  GrafPtr		origPort;
  bool			testNextEvent = true;

#if TARGET_API_MAC_CARBON
  if (pwQuitFlag) (*exitCB)();		// quit!
#endif

  while ( testNextEvent )
  {
	WaitNextEvent(everyEvent, &event, sleepTime, nil) ;
	
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
    	case autoKey:
    		updown = PW_DOWN ;
    	case keyUp:
		{
    		// handle key_events:
    		int ch	= event.message & charCodeMask ;		// character pressed
    		int key	= (event.message & keyCodeMask) >> 8 ;	// key pressed
    		ch = translateKey(ch, key);
    		GetPort(&origPort);
    		SETPORT(pwWindow);
    		GlobalToLocal(&event.where);
    		if (kbCB) (*kbCB) ( ch, updown, event.where.h, event.where.v ) ;
    		SetPort(origPort);
    		return;
    	}
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
				/*
				case inGrow:
					handleGrow(window, event.where);
					return;
				case inZoomIn:
				case inZoomOut:
					handleZoom(window, event.where, part);
					return;
				*/
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
    		break;

		case osEvt:
		{
			unsigned char subcode = event.message >> 24;
			if (subcode == suspendResumeMessage)   	//	Suspend/resume event	
				if (resumeFlag & event.message)		//	Resume
				{
					sleepTime = ACTIVE_SLEEPTIME;
					return;
				}
				else	// suspend
					sleepTime = INACTIVE_SLEEPTIME;
			break;
		}
		case activateEvt:
			if (event.modifiers & activeFlag)
			{ sleepTime = ACTIVE_SLEEPTIME; return; }
			else sleepTime = INACTIVE_SLEEPTIME;
			break;
		
		case updateEvt:
		case nullEvent:	// likely an idle event (i.e. no events)
					// report mouse location, only if mouse has moved:
					// (there is no mouse move events on Mac!)
		{
			int m_h = event.where.h;
			int m_v = event.where.v;
			if ( (m_v != last_m_v) || (m_h != last_m_h) )
			{
				GetPort(&origPort);
				SETPORT(pwWindow);
				GlobalToLocal(&event.where);
				last_m_v = m_v ;    last_m_h = m_h ;
				if ( mpCB )  (*mpCB) ( m_h, m_v ) ;
				SetPort(origPort);
			}
	        return;
		}
		
		default : /* printf ("event: %i \n", event.what); */ break;

    }		// switch(event.what)
  }		// while ( testNextEvent )
}


void pwInit ( int multisample, int num_samples )
{
	/* Open a full-screen window here please. */
	// should init DrawSprocket on OS 8.6 to 9 ...
	//err = DSpStartup ();
	//if (err != noErr)  logOut("DSpStartup error.\n");
#if TARGET_API_MAC_CARBON
	pwInit ( 0,0,-1,-1, multisample, "", 0, num_samples ) ;
#else
	ulSetError(UL_FATAL, "fullscreen not implemented with os < X \n");
	exit(3);
#endif
}


void pwInit ( int x, int y, int w, int h, int multisample,
              char *title, int border, int num_samples )
{
  if (pwInitialized)
  {
	ulSetError(UL_WARNING, "pwInit already called");
	return;
  }
  
  bool full_screen = ( (w<0) || (h<0) ) ? true : false ;
  Initialize();
  pwWindow = NULL;
  
  CreateContext(multisample, num_samples, full_screen);
  
#if TARGET_API_MAC_CARBON
  if (full_screen) {
	// Initialize OpenGL stuff (format, context, ...):
	aglSetCurrentContext(currContext);
	aglSetFullScreen (currContext, 0, 0, 0, 0);
  } else
#endif
  {
  	MakeWindow(x,y,w,h, title);
  	// Attach the context to the window
  	aglSetDrawable(currContext, GetWindowPort (GRAFPTR pwWindow));
#if !TARGET_API_MAC_CARBON
  	{	// because aglSetDrawable is slow... (not needed with OSX)
		EventRecord event;
		WaitNextEvent (everyEvent, &event, 2, NULL);
  	}
#endif
	aglSetCurrentContext(currContext);

	// create a default menu bar (with the "quit" command):
	MakeMenu();
  }
  pwInitialized = true ; 
}


void pwGetSize ( int *w, int *h )
{
	*w = size[0];
	*h = size[1];
}

//special cursors:
static Cursor q_curs =
{
	{0x0380, 0x07c0, 0x0c60, 0x1830, 0x1830, 0x0030, 0x0060, 0x00c0,
	 0x0180, 0x0180, 0x0180, 0, 0, 0x0180, 0x0180, 0},	// data
	{0x0380, 0x07e0, 0x0ff0, 0x1e38, 0x1c38, 0x1c38, 0x0078, 0x00f0,
	 0x01e0, 0x01c0, 0x01c0, 0x01c0, 0, 0x01c0, 0x01c0, 0x01c0},	// mask
	{ 14, 7 } 	// hotspot
};
static Cursor c_curs =
{
	{0, 0x03e0, 0x0c18, 0x1004, 0x2002, 0x2002, 0x4001, 0x4001,
	 0x4001, 0x4001, 0x4001, 0x2002, 0x2002, 0x1004, 0x0c18, 0x03e0},	// data
	{0, 0x03e0, 0x0ff8, 0x1c1c, 0x3006, 0x3006, 0x6003, 0x6003,
	 0x6003, 0x6003, 0x6003, 0x3006, 0x3006, 0x1c1c, 0x0ff8, 0x03e0},	// mask
	{ 8, 8 } 	// hotspot
};
static Cursor r_curs =
{
	{0, 0x0008, 0x0018, 0x0038, 0x0078, 0x00f8, 0x01f8, 0x03f8,
	 0x07f8, 0x00f8, 0x00d8, 0x0188, 0x0180, 0x0300, 0x0300, 0},	 // data
	{0x000c, 0x001c, 0x003c, 0x007c, 0x00fc, 0x01fc, 0x03fc, 0x07fc,
	 0x0ffc, 0x0ffc, 0x01fc, 0x03dc, 0x03cc, 0x0780, 0x0780, 0x0700}, // mask
	{ 1, 12 } 	// hotspot
};
static Cursor x_curs =
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
		case PW_CURSOR_AIM    : csrPtr = &x_curs; break; // ???
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
	
	pwSetOrigin( x, y );
	pwSetSize( w, h );
}


void pwSwapBuffers ()
{
	if (!pwInitialized)
	{
		ulSetError(UL_FATAL, "call to pwSwapBuffers without call to pwInit");
		exit(1);
	}
	// glFlush () ;
	aglSwapBuffers ( currContext ) ;
	pwHandleEvents () ;
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
}


#endif

