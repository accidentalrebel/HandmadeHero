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
	MessageBox(0, "This is Handmade Hero.", "Handmade Hero",
						 MB_OK|MB_ICONINFORMATION);

	WNDCLASSA windowClass = {};

	windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = MainWindowProcCallback;
  windowClass.hInstance = hInstance;
  // windowClass.hIcon = ;
  windowClass.lpszClassName = "HandmadeHeroWindowClass";
	
	return(0);
}
