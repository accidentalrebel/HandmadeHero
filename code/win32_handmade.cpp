#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool gIsRunning;

global_variable BITMAPINFO gBitmapInfo;
global_variable void* gBitmapMemory;
global_variable int gBitmapWidth;
global_variable int gBitmapHeight;
global_variable int gBytesPerPixel = 4;

internal void
RenderWeirdGradient(int xOffset, int yOffset)
{
	int pitch = gBitmapWidth * gBytesPerPixel;
	uint8_t* row = (uint8_t*)gBitmapMemory;
	for ( int y = 0; y < gBitmapHeight ; ++y )
	{
		uint32_t* pixel = (uint32_t*)row;
		for ( int x = 0; x < gBitmapWidth ; ++x )
		{
			uint8_t blue = (x + xOffset);
			uint8_t green = (y + yOffset);
			
			*pixel++ = ((green << 8) | blue);
		}

		row += pitch;
	}
}

internal void
Win32ResizeDIBSection(int width, int height)
{
	if ( gBitmapMemory )
	{
		VirtualFree(gBitmapMemory, 0, MEM_RELEASE);
	}

	gBitmapWidth = width;
	gBitmapHeight = height;
	
	gBitmapInfo.bmiHeader.biSize = sizeof(gBitmapInfo.bmiHeader);
	gBitmapInfo.bmiHeader.biWidth = gBitmapWidth;
	gBitmapInfo.bmiHeader.biHeight = -gBitmapHeight;
	gBitmapInfo.bmiHeader.biPlanes = 1;
	gBitmapInfo.bmiHeader.biBitCount = 32;
	gBitmapInfo.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = (gBitmapWidth*gBitmapHeight)*gBytesPerPixel;
	gBitmapMemory = VirtualAlloc(0,
															 bitmapMemorySize,
															 MEM_COMMIT,
															 PAGE_READWRITE);

	RenderWeirdGradient(0, 0);
}

internal void
Win32UpdateWindow(HDC deviceContext, RECT* clientRect, int x, int y, int width, int height)
{
	int windowWidth = clientRect->right - clientRect->left;
	int windowHeight = clientRect->bottom - clientRect->top;
	StretchDIBits(deviceContext,
								0, 0, gBitmapWidth, gBitmapHeight,
								0, 0, windowWidth, windowHeight,
								gBitmapMemory,
								&gBitmapInfo,
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
		 RECT clientRect;
		 GetClientRect(window, &clientRect);

		 int width = clientRect.right - clientRect.left;
		 int height = clientRect.bottom - clientRect.top;
		 
		 Win32ResizeDIBSection(width, height);
		 OutputDebugStringA("WM_SIZE\n");
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

		 RECT clientRect;
		 GetClientRect(window, &clientRect);
		 
		 Win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
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
	WNDCLASSA windowClass = {};
	windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
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
			gIsRunning = true;

			int xOffset = 0;
			int yOffset = 0;
			
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

				RenderWeirdGradient(xOffset, yOffset);
				HDC deviceContext = GetDC(window);
				RECT clientRect;
				GetClientRect(window, &clientRect);
				
				int windowWidth = clientRect.right - clientRect.left;
				int windowHeight = clientRect.bottom - clientRect.top;
					
				Win32UpdateWindow(deviceContext, &clientRect, 0, 0, windowWidth, windowHeight);
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
