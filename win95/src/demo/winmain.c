/*
 * winmain.c
 *
 * This code is derived from the Window 'main' handler that
 * is part of the 3DFX Glide 2.43 SDK. Used with permission
 * from 3DFx Interactive, the following is the original
 * copyright.
 *
** Copyright (c) 1997, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 1.2 $ 
** $Date: 1999-10-31 11:24:09-08 $ 
**
 */

#include <stdio.h>

/* This segment simulates main() for Windows, creates a window, etc. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include "p3dc.h"

/* Forward declarations */
HWND hWndMain;
char ** commandLineToArgv(LPSTR lpCmdLine, int *pArgc);

extern int p3dc_display_ctrl(int, ...);

/*
 * MainWndproc
 *
 * Callback for all Windows messages
 */
static int qhead = 0;
static int qtail = 0;
static int queue[256] = {0};

long FAR PASCAL 
MainWndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  PAINTSTRUCT ps;
  HDC         hdc;

//  p3dc_log("Got a message .. %d\n", message);
//  p3dc_log_flush();
  switch( message )
    {
    case WM_SETCURSOR:
      break;

    case WM_CREATE:
      break;

    case WM_PAINT:
      hdc = BeginPaint( hWnd, &ps );
      EndPaint( hWnd, &ps );
      return 1;

    case WM_CLOSE:
      queue[qhead++] = 'q'; qhead &= 255;
      break;

    case WM_DESTROY:
      break;

    case WM_MOVE:
      if (p3dc_display_ctrl(P3DC_CTRL_DISPLAY, P3DC_DISPLAY_MOVE)) {
        PostMessage( hWndMain, WM_CLOSE, 0, 0 );
        return 0;
      }
      break;

    case WM_ACTIVATE:
       if (wParam & WA_INACTIVE) 
          p3dc_display_ctrl(P3DC_CTRL_DISPLAY, P3DC_DISPLAY_OFF);
       else 
          p3dc_display_ctrl(P3DC_CTRL_DISPLAY, P3DC_DISPLAY_ON);
      break;

    case WM_DISPLAYCHANGE:
    case WM_SIZE:
      if (p3dc_display_ctrl(P3DC_CTRL_DISPLAY, P3DC_DISPLAY_RESIZE)) {
        MessageBox( hWnd, "Resize failed due to lack of sufficient buffer memory.\n", "Allocation Failure", MB_OK | MB_APPLMODAL );
        PostMessage( hWndMain, WM_CLOSE, 0, 0 );
        return 0;
      }
      break;

    case WM_CHAR:
		if (!isascii(wParam)) {
			p3dc_log("Got character %d\n", wParam);
			break;
		}
      /* queue[qhead++] = tolower(wParam);  */
      queue[qhead++] = wParam;
      qhead &= 255;
      break;

    default:
      break;
    }
  return DefWindowProc(hWnd, message, wParam, lParam);
  
} /* MainWndproc */

/*
 * initApplication
 *
 * Do that Windows initialization stuff...
 */
static BOOL
initApplication( HANDLE hInstance, int nCmdShow )
{
  WNDCLASS    wc;
  BOOL        rc;
  
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = MainWndproc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon( NULL, IDI_APPLICATION);    /* generic icon */
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = GetStockObject( BLACK_BRUSH );
  wc.lpszMenuName =  NULL;
  wc.lpszClassName = "WinGlideClass";
  rc = RegisterClass( &wc );
  if( !rc ) {
    return FALSE;
  }
  
  hWndMain =
    CreateWindowEx(
#if 0                         
                   WS_EX_APPWINDOW  gives you regular borders?
                   WS_EX_TOPMOST    Works as advertised.
#endif
                   WS_EX_APPWINDOW,
                   "WinGlideClass",
                   "Generic 3D API Test Application",
                   WS_OVERLAPPED |     
                   WS_CAPTION  |     
                   WS_THICKFRAME | 
                   WS_MAXIMIZEBOX | 
                   WS_MINIMIZEBOX | 
                   WS_VISIBLE | /* so we don't have to call ShowWindow */
                   WS_POPUP | /* non-app window */
                   WS_SYSMENU, /* so we get an icon in the tray */
                   CW_USEDEFAULT, 
                   CW_USEDEFAULT,
                   0x110, /* GetSystemMetrics(SM_CXSCREEN), */
                   0x120, /* GetSystemMetrics(SM_CYSCREEN), */
                   NULL,
                   NULL,
                   hInstance,
                   NULL
                   );
  
  if( !hWndMain ) {
    return FALSE;
  }
  
  SetCursor(NULL);
  
  ShowWindow( hWndMain, SW_NORMAL);
  UpdateWindow( hWndMain );
  
  return TRUE;
  
} /* initApplication */

/*
 * Converts lpCmdLine to WinMain into argc, argv
 */
static char    *fakeName = "WinTest";
static char    *argvbuf[32];
static char    cmdLineBuffer[1024];
char **
commandLineToArgv(LPSTR lpCmdLine, int *pArgc)
{
  char    *p, *pEnd;
  int     argc = 0;
  
  argvbuf[argc++] = fakeName;
  
  if (lpCmdLine == NULL) {
    *pArgc = argc;
    return argvbuf;
  }
  
  strcpy(cmdLineBuffer, lpCmdLine);
  p = cmdLineBuffer;
  pEnd = p + strlen(cmdLineBuffer);
  if (pEnd >= &cmdLineBuffer[1022]) pEnd = &cmdLineBuffer[1022];
  
  // fflush(stdout);
  
  while (1) {
    /* skip over white space */
    // fflush(stdout);

    while (*p == ' ') p++;
    if (p >= pEnd) break;

    argvbuf[argc++] = p;
    if (argc >= 32) break;

    /* skip till there's a 0 or a white space */
    while (*p && (*p != ' ')) p++;

    if (*p == ' ') *p++ = 0;
  }
  
  *pArgc = argc;
  return argvbuf;
}

/*
 * WinMain
 */
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow )
{
  if( !initApplication(hInstance, nCmdShow) )
    return FALSE;
  
  {
    /* 
     * Since printfs go into the bit bucket on Win32,
     * put up a message in the window.
     */
    HDC hDC = GetDC(hWndMain);
    char *message = "Press any Key to continue!";
    RECT rect;

    GetClientRect(hWndMain, &rect);
    SetTextColor(hDC, RGB(0, 255, 255));
    SetBkColor(hDC, RGB(0, 0, 0));
    SetTextAlign(hDC, TA_CENTER);
    ExtTextOut(hDC, rect.right/2, rect.bottom/2, ETO_OPAQUE, &rect, 
               message, strlen(message), NULL);
    ReleaseDC(hWndMain, hDC);
    GdiFlush();
  }
  
  {
    int     argc;
    char    **argv;
    extern int main(int argc, char **argv);

    argv = commandLineToArgv(lpCmdLine, &argc);
    main(argc, argv);
  }
  p3dc_close();  
  printf("Exiting winMain()\n");
  fflush(stdout);
  
  DestroyWindow(hWndMain);
  return 0;
  
} /* WinMain */

/*
LRESULT CALLBACK
KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
	WORD wInfo = (WORD) (lParam >> 16);
	BOOL fDown = ! (wInfo & KF_UP);
	BOOL fRepeat = wInfo & KF_REPEAT;

	if ((wParam == VK_DELETE) && (winfo & KF_ALTDOWN) && 
		(GetAsyncKeyState(VK_CONTROL) & 0x80000000))
		return CallNextHookEx(hh, code, wParam, lParam);

	if ((wParam == VK_ESCAPE) && (GetAsyncKeyState(VK_CONTROL) & 0x80000000))
		return CallNextHookEx(hh, code, wParam, lParam);

	if ((wParam == VK_TAB) && (wInfo & KF_ALTDOWN))
		return CallNextHookEx(hh, code, wParam, lParam);

	return CallNextHookEx(hh, code, wParam, lParam);
}
*/
	
int
next_char( void ) 
{
	MSG     msg;
	char    rv;
  

	if (qtail != qhead) {
		rv = queue[qtail++];
		qtail &= 255;
		return rv;
	}
  
	while (PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE)) {
		GetMessage( &msg, NULL, 0, 0 );
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (qtail != qhead) {
			rv = queue[qtail++];
			qtail &= 255;
			return rv;
		}
	}
	return -1;
}