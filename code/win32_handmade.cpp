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

struct win32_offscreen_buffer 
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

// Global for now
global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackbuffer;

struct win32_window_dimension
{
    int Width;
    int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return(Result);
}

internal void RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffSet, int YOffSet)
{
    uint8 *Row = (uint8 *)Buffer.Memory; 
    for(int Y = 0;
        Y < Buffer.Height;
        Y++)
    {
        uint8 *Pixel = (uint8 *)Row;
        for(int X = 0;
            X < Buffer.Width;
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
        Row += Buffer.Pitch;
    } 
}

// Define DIB Section Resize

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{

    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // positive start lower left, negative start upper left
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    //TODO : probably clear this to black
    Buffer->Pitch = Buffer->Width*Buffer->BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer Buffer, int X, int Y, int Width, int Height)
{
    // TODO: Aspect ratio correction
    StretchDIBits(  DeviceContext, 
                    /*X, Y, Width, Height,
                    X, Y, Width, Height,*/
                    0, 0, WindowWidth, WindowHeight,
                    0, 0, Buffer.Width, Buffer.Height,
                    Buffer.Memory,
                    &Buffer.Info,
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

            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer, X, Y, Width, Height);
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

    //win32_window_dimension Dimension = Win32GetWindowDimension(Window);
    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
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
                RenderWeirdGradient(GlobalBackbuffer, XOffSet, YOffSet);

                HDC DeviceContext = GetDC(Window);
                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer, 0, 0, Dimension.Width, Dimension.Height);
                ReleaseDC(Window, DeviceContext);
                //XOffSet++;
                YOffSet+=1;
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



