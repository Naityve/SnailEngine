/*
    Author : Naityve
    Date : 04/June/2023
    Email : naityve@gmail.com

    Following Handmade Hero Series by Molly Rocket / Casey Muratori.

    Notice : Any revisions or additions upon the original Molly Rocket source code
    (C) Copyright 2023 by Kacper (Naityve) Florek
*/

#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK
MainWindowCallback( HWND Window,
                    UINT Message,
                    WPARAM WParam,
                    LPARAM LParam)
{
    switch(Message)
    {
        case WM_SIZE:
        {
        } break;

        case WM_DESTROY:
        {

        } break;

        case WM_CLOSE:
        {

        } break;

        case WM_ACTIVATEAPP:
        {

        } break;

        default:
        {

        } break;
    }
}

// Windows Desktop Window Declaration

/*
typedef struct tagWNDCLASS {
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCTSTR lpszMenuName;
    LPCTSTR lpszClassName;
} WNDCLASS, *PWNDCLASS;
*/

// nCmdShow is the size the cmd window is set to. Default, minimized, maximized. 

int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CommandLine,
                     int ShowCode)
{
    WNDCLASS WindowClass = {};

    // set WNDCLASS properties

    // CS_OWNDC : Allocates a unique device context for each window in the class.
    // CS_HREDRAW : Redraws the entire window if a movement or 
    //              size adjustment changes the width of the client area
    // CS_VREDRAW : Redraws the entire window if a movement or
    //              size adjustment changes the height of the client area
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW; 

    // lpfnWndProc : A pointer to the window procedure.
    WindowClass.lpfnWndProc = ;
    WindowClass.hInstance = Instance;
//  WindowClass.hIcon
    WindowClass.lpszClassName = "SnailEngineWindowClass";

    // MessageBox(0, "Hello Snail!","SnailEngine", MB_OK|MB_ICONINFORMATION );
    return(0);
}



