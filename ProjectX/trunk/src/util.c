#include <stdio.h>
#include <stdarg.h>
#include "main.h"
#include "file.h"
#include "util.h"
#include "string.h"

extern BOOL Debug;
BOOL DebugLog = FALSE;

#ifdef WIN32
#include <windows.h>	// for various things
#include <ctype.h>		// for toupper
#endif

void strtoupper(char *str)
{
	while (*str)
	{
		*str = (char) toupper(*str); 
		str++; 
	}
}

void GetFilename( uint8 * Src, uint8 * Dest )
{
	uint8	*	Char_Ptr;

	Char_Ptr = ( Src + strlen( Src ) ) -1;

	while( Char_Ptr != Src && *Char_Ptr != '\\' && *Char_Ptr != ':' ) Char_Ptr--;

	if( Char_Ptr == Src )
	{
		strcpy( Dest, Src );
		return;
	}

	if( Char_Ptr != ( Src + strlen( Src ) - 1 ) )
	{
		strcpy( Dest, ( Char_Ptr + 1 ) );
	}
	else
	{
		*Dest = 0;
	}
}

void Get_Ext( uint8 * Src, uint8 * Dest )
{
	uint8	*	Char_Ptr;

	Char_Ptr = ( Src + strlen( Src ) ) -1;

	while( Char_Ptr != Src && *Char_Ptr != '\\' && *Char_Ptr != ':' && *Char_Ptr != '.' ) Char_Ptr--;

	if( *Char_Ptr == '.' )
	{
		Char_Ptr++;
		while( *Char_Ptr ) *Dest++ = *Char_Ptr++;
		*Dest = 0;
	}
	else
	{
		*Dest = 0;
	}
}

void Add_Path( uint8 * Path, uint8 * Src, uint8 * Dest )
{
	strcpy( Dest, Path );
	Dest = ( Dest + strlen( Path ) );
	strcpy( Dest, Src );
}

void Change_Ext( const char * Src, char * Dest, const char * Ext )
{
	uint8	*	Char_Ptr;

	int length = strlen( Src );

	if ( ! length )
	{
		strcpy( Dest, Ext ); // set the extension
		Msg("Change_Ext called with Src empty!");
		return;
	}

	Char_Ptr = ( (uint8*)Src + length ) - 1;

	while( Char_Ptr != Src && *Char_Ptr != '\\' && *Char_Ptr != ':' && *Char_Ptr != '.' ) Char_Ptr--;

	if( *Char_Ptr == '.' )
	{
		while( Src != Char_Ptr )
			*Dest++ = *Src++;
		strcpy( Dest, Ext );
	}
	else
	{
		strcpy( Dest, Src );
		Dest += strlen( Src );
		strcpy( Dest, Ext );
	}
}

void DebugPrintf( const char * format, ... )
{

  static char buf1[256], buf2[512];
  va_list args;

  // command line switch
  if ( ! Debug )
    return;

  va_start( args, format );
  vsprintf( buf1, format, args );
  sprintf( buf2, "%hs", buf1 );

#ifdef WIN32
  OutputDebugString( buf2 );
#else
  printf( buf2 );
#endif

  va_end( args );

  // add the comment to the log file
  if( DebugLog )
	AddCommentToLog( buf2 );

}

// prints a message only if it wasn't the last one to be printed...
// this way you can debug game state without getting a message every single loop
// for instance in the title screen if you print "in title screen" it will only print once
// instead of every single loop....
void DebugState( const char * str )
{
	static const char * last;
	if ( !Debug ) 
		return;
	if ( last && strcmp( str, last ) == 0 )
		return;
	last = str;
	DebugPrintf( str );
}

//
// Creates a message box and returns ok/cancel press
//

// temporarily jumps to desktop mode
#include "render.h"
#include "title.h"
extern render_info_t render_info;
extern void MenuGoFullScreen( MENUITEM *Item );
extern void input_grab( BOOL clip );
int Msg( const char * msg, ... )
{
	BOOL was_fullscreen = render_info.bFullscreen;

	char txt[ 1024 ];
	va_list args;
	int res = 0;

	va_start( args, msg );
	vsprintf( txt, msg, args);
	va_end( args );

#ifdef WIN32

    if (render_info.bFullscreen)
	{
		// switch to window mode
		// other wise pop up will get stuck behind main window
		MenuGoFullScreen(NULL);
		// push main window to background so popup shows
        SetWindowPos(GetActiveWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	
	// release mouse so they can interact with message box
	input_grab( FALSE );

    res = MessageBox( GetActiveWindow(), txt, "Forsaken", MB_OKCANCEL | MB_ICONEXCLAMATION );

    if (was_fullscreen)
	{
		// switch back to fullscreen
		MenuGoFullScreen(NULL);
        SetWindowPos(GetActiveWindow(), HWND_TOPMOST, 0, 0, 0, 0,  SWP_NOSIZE | SWP_NOMOVE);
		input_grab( TRUE ); // don't do this in window mode just let them click back on the window
	}

#endif

	DebugPrintf( txt );

	// IDCANCEL	Cancel button was selected.
	// IDOK	OK button was selected.
	return res;
}
