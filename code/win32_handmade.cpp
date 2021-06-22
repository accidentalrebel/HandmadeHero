#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool gIsRunning;
global_variable BITMAPINFO gBitmapInfo;
global_variable void *gBitmapMemory;
global_variable HBITMAP gBitmapHandle;
global_variable HDC gBitmapDeviceContext;

internal void
Win32ResizeDIBSection(int width, int height)
{
	if ( gBitmapHandle )
	{
		DeleteObject(gBitmapHandle);
	}
	if ( !gBitmapDeviceContext )
	{
		gBitmapDeviceContext = CreateCompatibleDC(0);
	}
	
	gBitmapInfo.bmiHeader.biSize = sizeof(gBitmapInfo.bmiHeader);
	gBitmapInfo.bmiHeader.biWidth = width;
	gBitmapInfo.bmiHeader.biHeight = height;
	gBitmapInfo.bmiHeader.biPlanes = 1;
	gBitmapInfo.bmiHeader.biBitCount = 32;
	gBitmapInfo.bmiHeader.biCompression = BI_RGB;
	
  gBitmapHandle = CreateDIBSection(gBitmapDeviceContext,
																	 &gBitmapInfo,
																	 DIB_RGB_COLORS,
																	 &gBitmapMemory,
																	 0, 0);
}

internal void
Win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height)
{
	StretchDIBits(deviceContext,
										x, y, width, height,
										x, y, width, height,
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

		 Win32UpdateWindow(deviceContext, x, y, width, height);
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
		HWND windowHandle = CreateWindowExA(
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
		if ( windowHandle )
		{
			gIsRunning = true;
			
			while(gIsRunning)
			{
				MSG message;
				BOOL messageResult = GetMessageA(&message, 0, 0, 0);
				if ( messageResult > 0 )
				{
					TranslateMessage(&message);
					DispatchMessageA(&message);
				}
				else
				{
					break;
				}
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
