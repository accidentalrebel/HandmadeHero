#include <windows.h>

#define internal_function static
#define local_persist static
#define global_variable static

global_variable bool gIsRunning;

LRESULT CALLBACK
MainWindowProcCallback(HWND window,
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
		 local_persist DWORD operation = WHITENESS;
		 if ( operation == WHITENESS )
			 operation = BLACKNESS;
		 else
			 operation = WHITENESS;
		 PatBlt(deviceContext, x, y, width, height, operation);
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
	windowClass.lpfnWndProc = MainWindowProcCallback;
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
