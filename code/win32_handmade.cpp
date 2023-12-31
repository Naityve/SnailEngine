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
#include <xinput.h>
#include <dsound.h>

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

typedef int32 bool32;

struct win32_offscreen_buffer 
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

// Global for now
global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;

// Testing Globals
global_variable bool inputTest;

struct win32_window_dimension
{
    int Width;
    int Height;
};

// Note: XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
//#define XInputGetState XInputGetState_;

// Note: XInputSetState

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
//#define XInputSetState XInputSetState_; // does not compile in c++, only in C99. fuck you stroustrup  

internal void Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll"); // for older version of win32
    if(!XInputLibrary)
    {
        // diagnostic
        XInputLibrary = LoadLibraryA("xinput1_4.dll"); // because we're on newest windows "xinput1_3.dll" does not work
    }
    if(XInputLibrary)
    {
        XInputGetState_ = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        //if(!XInputGetState_) {XInputGetState = XInputGetStateStub;}
        XInputSetState_ = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        //if(!XInputSetState_) {XInputSetState = XInputSetStateStub;}
    }
    else
    {
        // diagnostic
    }
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name (LPCGUID pcGuidDDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // load the library dsound
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if(DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        IDirectSound *DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                // create primary buffer
                // TODO check if you need DSCAPS_GLOBALFOCUS
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                    {
                        OutputDebugStringA("Primary buffer format was set up \n");
                    }
                    else
                    {
                        // diagnostic
                    }
                }
            }
            else
            {
                // diagnostic
            }

            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            LPDIRECTSOUNDBUFFER SecondaryBuffer;
            if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0)))
            {
                // start it playing
                OutputDebugStringA("Secondary buffer format was set up \n");
            }
            
        }
        else
        {
            //diagnostic
        }
    }
    


}

internal win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return(Result);
}

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffSet, int YOffSet)
{
    uint8 *Row = (uint8 *)Buffer->Memory; 
    for(int Y = 0;
        Y < Buffer->Height;
        Y++)
    {
        uint8 *Pixel = (uint8 *)Row;
        for(int X = 0;
            X < Buffer->Width;
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
        Row += Buffer->Pitch;
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
    int BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // positive start lower left, negative start upper left
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    //TODO : probably clear this to black
    Buffer->Pitch = Buffer->Width*BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer *Buffer)
{
    // TODO: Aspect ratio correction
    StretchDIBits(  DeviceContext, 
                    /*X, Y, Width, Height,
                    X, Y, Width, Height,*/
                    0, 0, WindowWidth, WindowHeight,
                    0, 0, Buffer->Width, Buffer->Height,
                    Buffer->Memory,
                    &Buffer->Info,
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
            GlobalRunning = false;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            inputTest = false;

            uint32 VKCode = WParam;
            bool WasDown = ((LParam & (1 << 30)) != 0); // check if key was down or up before being pressed
            bool IsDown = ((LParam & (1 << 30)) == 0); // checks if a key is down
            if(VKCode == 0x57) // up "W"
            {

            }
            else if(VKCode == 0x41) // left "A"
            {

            }
            else if(VKCode == 0x53) // down "S"
            {
                
            }
            else if(VKCode == 0x44) // right "D"
            {
                
            }
            else if(VKCode == 0x51) // utility "Q"
            {
                
            }
            else if(VKCode == 0x45) // heal "E"
            {
                
            }
            else if(VKCode == 0x52) // reload "R"
            {
                
            }
            else if(VKCode == 0x46) // interact "F" 0x46
            {
                inputTest = true;
            }
            else if(VKCode == 0x47) // drop things "G"
            {
                
            }
            else if(VKCode == 0x4A) // journal "J"
            {
                
            }
            else if(VKCode == VK_SPACE) // jump
            {
                
            }
            else if(VKCode == VK_CONTROL) // crouch
            {
                
            }
            else if(VKCode == VK_UP) // arrow key up
            {
                
            }else if(VKCode == VK_LEFT) // arrow key left
            {
                
            }else if(VKCode == VK_DOWN) // arrow key down
            {
                
            }else if(VKCode == VK_RIGHT) // arrow key right
            {
                
            }

            bool32 AltKeyWasDown = (LParam & (1 << 29));
            if((VKCode == VK_F4) && AltKeyWasDown)
            {
                GlobalRunning = false;
            }
        } break;

        case WM_CLOSE:
        {   
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP \n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackbuffer);
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
    Win32LoadXInput();

    WNDCLASSA WindowClass = {};

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
            GlobalRunning = true;
            int XOffSet = 0;
            int YOffSet = 0;
            Win32InitDSound(Window, 48000, 48000*sizeof(int16)*2);
            while(GlobalRunning)
            {
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                for(DWORD ControllerIndex = 0;
                    ControllerIndex < XUSER_MAX_COUNT;
                    ++ControllerIndex)
                {
                    XINPUT_STATE ControllerState;
                    if(XInputGetState_( ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        // controller plugged in
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad; 

                        // Controller Mapping
                        // DPAD = arrow buttons
                        // sThumbLX/Y = Left thumb stick
                        // sThumbRX/Y = right thumb stick
                        // XINPUT_GAMEPAD_A/B/X/Y = Buttons

                        bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);
                    
                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;

                        
                        YOffSet+=StickY >> 16;
                        XOffSet-=StickX >> 16; // reverse the direction so pointing stick left moves texture left
                        
                    } 
                    else 
                    {
                        // controller not connected
                       
                    }
                }

                RenderWeirdGradient(&GlobalBackbuffer, XOffSet, YOffSet);

                HDC DeviceContext = GetDC(Window);
                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackbuffer);
                ReleaseDC(Window, DeviceContext);
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



