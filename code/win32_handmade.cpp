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
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// Global for now
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;

global_variable int BitmapWidth;
global_variable int BitmapHeight;

global_variable int BytesPerPixel = 4;

internal void RenderWeirdGradient(int XOffSet, int YOffSet)
{
    int Width = BitmapWidth;
    int Height = BitmapHeight;
    int Pitch = Width*BytesPerPixel;
    uint8 *Row = (uint8 *)BitmapMemory; 
    for(int Y = 0;
        Y < BitmapHeight;
        Y++)
    {
        uint8 *Pixel = (uint8 *)Row;
        for(int X = 0;
            X < BitmapWidth;
            X++)
        {
            //*Pixel = 170;//blue
            *Pixel = (uint8) (X + XOffSet);
            Pixel++;

            //*Pixel = 50;//green
            *Pixel = (uint8) (Y + YOffSet);
            Pixel++;

            *Pixel = 100; //red
            Pixel++;

            *Pixel = 0; //pad
            Pixel++;
        }
        Row += Pitch;
    } 
}

// Define DIB Section Resize

internal void
Win32ResizeDIBSection(int Width, int Height)
{

    if(BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight; // positive start lower left, negative start upper left
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    //TODO : probably clear this to black
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
    /*

    */

    int WindowWidth = ClientRect->bottom - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(  DeviceContext, 
                    /*X, Y, Width, Height,
                    X, Y, Width, Height,*/
                    0, 0, BitmapWidth, BitmapHeight,
                    0, 0, WindowWidth, WindowHeight,
                    BitmapMemory,
                    &BitmapInfo,
                    DIB_RGB_COLORS, SRCCOPY);
}


// lpfnWndProc Declaration

LRESULT CALLBACK
Win32MainWindowCallback( HWND Window,
                    UINT Message,
                    WPARAM WParam,
                    LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {

        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(  Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break; 

        case WM_DESTROY:
        {
            Running = false;
        } break;

        case WM_CLOSE:
        {   
            Running = false;
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

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);

            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
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
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance; 
//  WindowClass.hIcon = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if(RegisterClass(&WindowClass))
    {
        HWND Window =
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
        if(Window)
        {
            Running = true;
            int XOffSet = 0;
            int YOffSet = 0;
            while(Running)
            {
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                RenderWeirdGradient(XOffSet, YOffSet);

                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);
                XOffSet++;
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



