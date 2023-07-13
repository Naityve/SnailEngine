/*
    Author : Naityve
    Date : 04/June/2023
    Email : naityve@gmail.com

    Following Handmade Hero Series by Molly Rocket / Casey Muratori.

    Notice : Any revisions or additions upon the original Molly Rocket source code
    (C) Copyright 2023 by Kacper (Naityve) Florek
*/

#include <windows.h>
#include <wingdi.h>
#include <stdio.h>


// lpfnWndProc Declaration

LRESULT CALLBACK
MainWindowCallback( HWND Window,
                    UINT Message,
                    WPARAM WParam,
                    LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {

        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE \n");
        } break; 

        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY \n");
        } break;

        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE \n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP \n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            static DWORD Operation = WHITENESS;
            PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
            if (Operation == WHITENESS)
            {
                Operation = BLACKNESS;
            }
            else
            {
                Operation = WHITENESS;
            }
            EndPaint(Window, &Paint);
        } break; 

        default:
        {
            //OutputDebugStringA("default \n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);

}

// MAIN FUNCTION

int CALLBACK 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{   
    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance; 
//  WindowClass.hIcon = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if(RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowEx(
                0,
                WindowClass.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                Instance,
                0
            );
        if(WindowHandle)
        {
            for(;;)
            {
                MSG Message;
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            // Logging Error Handling
        }
    }
    else
    {
        // Logging Error Handling
    }

    return(0);
}



