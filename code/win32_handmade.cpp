#include <windows.h>

// LRESULT LRESULT DefWindowProcA(
//   HWND   hWnd,
//   UINT   Msg,
//   WPARAM wParam,
//   LPARAM lParam
// );

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
		 OutputDebugStringA("WM_CLOSE\n");
	 } break;
	 
	 case WM_SIZE:
	 {
		 OutputDebugStringA("WM_SIZE\n");
	 } break;
	 
	 case WM_DESTROY:
	 {
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
		 static DWORD operation = WHITENESS;
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
			for(;;)
			{
				MSG message;
				BOOL messageResult = GetMessage(&message, 0, 0, 0);
				if ( messageResult > 0 )
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
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
