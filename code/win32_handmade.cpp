#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>

#define internal static
#define local_persist static
#define global_variable static

#define pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

struct Win32OffscreenBuffer
{
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int pitch;
	int bytesPerPixel;
};

struct Win32WindowDimension
{
	int width;
	int height;
};

struct Win32SoundOutput
{
	int samplesPerSecond;
	int toneHz;
	int16 toneVolume;
	uint32 runningSampleIndex;
	int wavePeriod;
	int bytesPerSample;
	int secondaryBufferSize;
};

global_variable bool gIsRunning;
global_variable Win32OffscreenBuffer gBackBuffer;
global_variable LPDIRECTSOUNDBUFFER gSecondaryBuffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32LoadXInput(void)
{
	HMODULE xInputLibrary = LoadLibraryA("xinput1_4.dll");
	if ( !xInputLibrary )
	{
		xInputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if ( xInputLibrary )
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(xInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary, "XInputSetState");
	}
	else
	{
		//TODO: Add diagnostic
	}
}

internal void
Win32InitDSound(HWND window, int32 samplesPerSecond, int32 bufferSize)
{
	HMODULE dSoundLibrary = LoadLibrary("dsound.dll");
	if ( dSoundLibrary )
	{
		direct_sound_create *directSoundCreate = (direct_sound_create*)GetProcAddress(dSoundLibrary, "DirectSoundCreate");

		LPDIRECTSOUND directSound;
		if ( directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0)))
		{
			WAVEFORMATEX waveFormat = {};
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nChannels = 2;
			waveFormat.nSamplesPerSec = samplesPerSecond;
			waveFormat.wBitsPerSample = 16;
			waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;
				
			if ( directSound->SetCooperativeLevel(window, DSSCL_PRIORITY) == DS_OK )
			{
				DSBUFFERDESC bufferDescription = {};
				bufferDescription.dwSize = sizeof(bufferDescription);
				bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

				LPDIRECTSOUNDBUFFER primaryBuffer;
				if ( SUCCEEDED(directSound->CreateSoundBuffer( &bufferDescription, &primaryBuffer, 0 )) )
				{					
					if( SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)) )
					{
						OutputDebugStringA("Primary buffer format was set.\n");
					}
					else
					{
						//TODO: Add diagnostic
					}
				}
				else
				{
					//TODO: Add diagnostic
				}
			}
			else
			{
				//TODO: Add diagnostic
			}

			DSBUFFERDESC bufferDescription = {};
			bufferDescription.dwSize = sizeof(bufferDescription);
			bufferDescription.dwFlags = 0;
			bufferDescription.dwBufferBytes = bufferSize;
			bufferDescription.lpwfxFormat = &waveFormat;

			HRESULT error = directSound->CreateSoundBuffer(&bufferDescription, &gSecondaryBuffer, 0);
			if (SUCCEEDED(error))
			{
				OutputDebugStringA("Secondary buffer was created successfully.\n");
			}
			else
			{
				//TODO: Add diagnostic
			}
		}
		else
		{
			//TODO: Add diagnostic
		}
	}
	else
	{
		//TODO: Add diagnostic
	}
}

internal Win32WindowDimension
GetWindowDimension(HWND window)
{
	Win32WindowDimension result;

	RECT clientRect;
	GetClientRect(window, &clientRect);
				
	result.width = clientRect.right - clientRect.left;
	result.height = clientRect.bottom - clientRect.top;

	return(result);
}
 
internal void
RenderWeirdGradient(Win32OffscreenBuffer *buffer, int xOffset, int yOffset)
{
	uint8* row = (uint8*)buffer->memory;
	for ( int y = 0; y < buffer->height ; ++y )
	{
		uint32_t* pixel = (uint32_t*)row;
		for ( int x = 0; x < buffer->width ; ++x )
		{
			uint8 blue = (x + xOffset);
			uint8 green = (y + yOffset);
			
			*pixel++ = ((green << 8) | blue);
		}

		row += buffer->pitch;
	}
}

internal void
Win32ResizeDIBSection(Win32OffscreenBuffer *buffer, int width, int height)
{
	if ( buffer->memory )
	{
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerPixel = 4;
	
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = (buffer->width*buffer->height)*buffer->bytesPerPixel;
	buffer->memory = VirtualAlloc(0,
								  bitmapMemorySize,
								  MEM_COMMIT,
								  PAGE_READWRITE);

	buffer->pitch = buffer->width * buffer->bytesPerPixel;

	RenderWeirdGradient(buffer, 0, 0);
}

internal void
Win32DisplayBufferInWindow(HDC deviceContext, int windowWidth, int windowHeight, Win32OffscreenBuffer *buffer, int x, int y, int width, int height)
{
	StretchDIBits(deviceContext,
				  0, 0, windowWidth, windowHeight,
				  0, 0, buffer->width, buffer->height,
				  buffer->memory,
				  &buffer->info,
				  DIB_RGB_COLORS,
				  SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowProcCallback(HWND window,
							UINT message,
							WPARAM wParam,
							LPARAM lParam)
{
	LRESULT result = 0;
	
	switch(message)
	{
	  case WM_ACTIVATEAPP:
	  {
		  OutputDebugStringA("WM_ACTIVATEAPP\n");
	  } break;
	 
	  case WM_CLOSE:
	  {
		  gIsRunning = false;
		  OutputDebugStringA("WM_CLOSE\n");
	  } break;
	 
	  case WM_SIZE:
	  {
		  OutputDebugStringA("WM_SIZE\n");
	  } break;

	  case WM_SYSKEYDOWN:
	  case WM_SYSKEYUP:
	  case WM_KEYDOWN:
	  case WM_KEYUP:
	  {
		  uint32 vkCode = wParam;
		  bool isDown = ((lParam & (1 << 31 )) == 0);
		  bool wasDown = ((lParam & ( 1 << 30 )) != 0);
		  if ( isDown != wasDown )
		  {
			  if ( vkCode == VK_ESCAPE )
			  {
				  OutputDebugStringA("ESCAPE: ");
				  if( isDown )
				  {
					  gIsRunning = false;
					  OutputDebugStringA("IsDown ");
				  }
				  if ( wasDown )
				  {
					  OutputDebugStringA("WasDown ");
				  }
				  OutputDebugStringA("\n");
			  }
			  else if ( vkCode == VK_SPACE )
			  {
			 
			  }
			  else if ( vkCode == VK_UP )
			  {
			 
			  }
			  else if ( vkCode == VK_RIGHT )
			  {
			 
			  }
			  else if ( vkCode == VK_DOWN )
			  {
			 
			  }
			  else if ( vkCode == VK_LEFT )
			  {
			 
			  }
		  }

		  bool32 altKeyWasDown = (lParam & ( 1 << 29 ));
		  if ( vkCode == VK_F4 && altKeyWasDown )
		  {
			  gIsRunning = false;
		  }

	  } break;
	 
	  case WM_DESTROY:
	  {
		  gIsRunning = false;
		  OutputDebugStringA("WM_DESTROY\n");
	  } break;

	  case WM_PAINT:
	  {
		  PAINTSTRUCT paint;
		  HDC deviceContext = BeginPaint(window, &paint);

		  int x = paint.rcPaint.left;
		  int y = paint.rcPaint.top;
		  int width = paint.rcPaint.right - paint.rcPaint.left;
		  int height = paint.rcPaint.bottom - paint.rcPaint.top;

		  Win32WindowDimension windowDimension = GetWindowDimension(window);
		  Win32DisplayBufferInWindow(deviceContext, windowDimension.width, windowDimension.height, &gBackBuffer, x, y, width, height);
		  EndPaint(window, &paint);
	  } break;
	 
	  default:
	  {
		  // OutputDebugStringA("WM Default");
		  result = DefWindowProcA(window, message, wParam, lParam);
	  } break;
	}

	return(result);
}

int CALLBACK
WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow)
{
	Win32LoadXInput();
	
	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MainWindowProcCallback;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = "HandmadeHeroWindowClass";

	if ( RegisterClass(&windowClass) )
	{
		HWND window = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0);
		if ( window )
		{
			Win32WindowDimension windowDimension = GetWindowDimension(window);
			Win32ResizeDIBSection(&gBackBuffer, 1280, 720);
			
			gIsRunning = true;

			// NOTE: Graphics test
			int xOffset = 0;
			int yOffset = 0;

			Win32SoundOutput soundOutput;
			soundOutput.samplesPerSecond = 48000;
			soundOutput.toneHz = 256;
			soundOutput.toneVolume = 1000;
			soundOutput.runningSampleIndex = 0;
			soundOutput.wavePeriod = soundOutput.samplesPerSecond / soundOutput.toneHz;
			soundOutput.bytesPerSample = sizeof(int16)*2;
			soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
			
			Win32InitDSound(window, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
			bool isSoundPlaying = false;
			
			while(gIsRunning)
			{
				MSG message;
				if ( PeekMessageA(&message, 0, 0, 0, PM_REMOVE) )
				{
					if ( message.message == WM_QUIT )
					{
						gIsRunning = false;
					}
					
					TranslateMessage(&message);
					DispatchMessageA(&message);
				}

				for ( DWORD controllerIndex =0 ; controllerIndex < XUSER_MAX_COUNT ; ++controllerIndex )
				{
					XINPUT_STATE controllerState;
					if ( XInputGetState(controllerIndex, &controllerState ) == ERROR_SUCCESS )
					{
						XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
						bool dpadUp = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool dpadRight = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool dpadDown = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool dpadLeft = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool dpadStart = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool dpadBack = (pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool dpadLeftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool dpadRigthShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool dpadA = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool dpadB = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool dpadX = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool dpadY = (pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 stickX = pad->sThumbLX;
						int16 stickY = pad->sThumbLY;

						if ( dpadA )
						{
							++yOffset;
						}
					}
					else
					{
						// OutputDebugString("Controller is not available.\n");
					}
				}

				RenderWeirdGradient(&gBackBuffer, xOffset, yOffset);

				DWORD playCursor;
				DWORD writeCursor;
				if ( SUCCEEDED(gSecondaryBuffer->GetCurrentPosition(&playCursor,
																	&writeCursor)) )
				{
					DWORD bytesToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize;
					DWORD bytesToWrite;
					if ( bytesToLock > playCursor )
					{
						bytesToWrite = soundOutput.secondaryBufferSize - bytesToLock;
						bytesToWrite += playCursor;
					}
					else
					{
						bytesToWrite = playCursor - bytesToLock;
					}

					VOID *region1;
					DWORD region1Size;
					VOID *region2;
					DWORD region2Size;

					if ( SUCCEEDED(gSecondaryBuffer->Lock(bytesToLock,
														  bytesToWrite,
														  &region1, &region1Size,
														  &region2, &region2Size,
														  0)) )
					{
						int16 *sampleOut = (int16 *)region1;
						DWORD region1SampleCount = region1Size/soundOutput.bytesPerSample;
						DWORD region2SampleCount = region2Size/soundOutput.bytesPerSample;
						for(DWORD sampleIndex = 0;
							sampleIndex < region1SampleCount;
							++sampleIndex)
						{
							real32 t = 2.0f * pi32 * (real32)soundOutput.runningSampleIndex / (real32)soundOutput.wavePeriod;
							real32 sineValue = sinf(t);
							int16 sampleValue = (int16)(sineValue * soundOutput.toneVolume);
							
							*sampleOut++ = sampleValue;
							*sampleOut++ = sampleValue;
							++soundOutput.runningSampleIndex;
						}

						sampleOut = (int16 *)region2;
						for(DWORD sampleIndex = 0;
							sampleIndex < region2SampleCount;
							++sampleIndex)
						{
							real32 t = 2.0f * pi32 * (real32)soundOutput.runningSampleIndex / (real32)soundOutput.wavePeriod;
							real32 sineValue = sinf(t);
							int16 sampleValue = (int16)(sineValue * soundOutput.toneVolume);
							
							*sampleOut++ = sampleValue;
							*sampleOut++ = sampleValue;
							++soundOutput.runningSampleIndex;
						}
					}

					gSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
				}

				if ( !isSoundPlaying )
				{
					gSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING );
					isSoundPlaying = true;
				}
				
				HDC deviceContext = GetDC(window);
				Win32WindowDimension windowDimension = GetWindowDimension(window);
					
				Win32DisplayBufferInWindow(deviceContext, windowDimension.width, windowDimension.height, &gBackBuffer, 0, 0, windowDimension.width, windowDimension.height);
				ReleaseDC(window, deviceContext);

				++xOffset;
			}
		} 
		else
		{
			//TODO: Log error
		}
	}
	else
	{
		// TODO: Log error
	}
	
	return(0);
}
