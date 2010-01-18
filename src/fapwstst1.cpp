//************************************************************
// The test of Finite Automata Programming environment (FAP)
// Yuri Borisov  15-Dec-08 FAPW_CR_011 Aligned with FAPWS FAPWS_REL_21SEP06
//************************************************************

#include "stdafx.h"
#include "resource.h"
#include "fapext.h"
#include "fapwstsob.h"

#define APPNAME "fapwstest1" 
#define APPTITLE "fapwstest1" 

// Makes it easier to determine appropriate code paths: 
#if defined (WIN32)    
#define IS_WIN32 TRUE 
#else    
#define IS_WIN32 FALSE 
#endif 

#define MAX_LOADSTRING 256
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000) 
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4)) 

HINSTANCE hInst;      // current instance 
char szAppName[100];  // Name of the app 
char szTitle[100];    // The title bar text 


class CFT_BArrea_Painter;

CAE_Env* fape = NULL;
CFT_Area* farea = NULL;
CFT_BArrea_Painter* fareapainter = NULL;


class CFT_BArrea_Painter: public MBallAreaWindow
{
public:
	CFT_BArrea_Painter(HWND aHwnd) : iHwnd(aHwnd) {}
	virtual ~CFT_BArrea_Painter() {}
	void beginPaint(HWND aHwnd);
	void endPaint();
private:
	//from MBallAreaWindow
	virtual void redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase);
	virtual void drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor);
	virtual CF_Rect boundaryRect();
private:
	HWND iHwnd;
	HWND iDrawHwnd;
	PAINTSTRUCT iPs;
	HDC iHdc;
};

void CFT_BArrea_Painter::beginPaint(HWND aHwnd)
{
	iDrawHwnd = aHwnd;
	iHdc = BeginPaint (iDrawHwnd, &iPs);          
	if (farea)
		farea->Draw();
}

void CFT_BArrea_Painter::endPaint()
{
	EndPaint(iDrawHwnd, &iPs);
}

void CFT_BArrea_Painter::redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase)
{
		RECT rect;
		rect.left = aCenter.iX-aRadius; if (rect.left < 0) rect.left = 0;
		rect.top = aCenter.iY-aRadius; if (rect.top < 0) rect.top = 0;
		rect.right = aCenter.iX+aRadius;
		rect.bottom = aCenter.iY+aRadius;
		InvalidateRect(iHwnd, &rect, aErase);
}

void CFT_BArrea_Painter::drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor)
{
	HBRUSH brush;
	long centx, centy;
	COLORREF  color = ((aColor.iRed & 0xff) << 16) | ((aColor.iGreen & 0xff) << 8) | (aColor.iBlue & 0xff);
	brush = CreateSolidBrush(color);
	SelectObject(iHdc, brush);
	centx = aCenter.iX;
	centy = aCenter.iY;
	Ellipse(iHdc, centx-aRadius, centy-aRadius, centx+aRadius, centy+aRadius);
	DeleteObject(brush);
}

CF_Rect CFT_BArrea_Painter::boundaryRect()
{
	RECT rt;
	GetClientRect(iHwnd, &rt);
	return CF_Rect(rt.left, rt.top, rt.right, rt.bottom);
}

const char* KLogSpecFileName = "C:\\LOGS\\FAP\\FAPLOGSPEC.TXT";

const char* KFAreaName = "Area";

const UINT KTimerEnv = 1;

// The FAP environment timer tick value, millisec
const UINT KTimerEnvElapsInt = 10;

BOOL SShiftModeOn = FALSE;
POINT StartShiftPosition;

BOOL InitApplication(HINSTANCE); 
BOOL InitInstance(HINSTANCE, int); 
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); 

void HandleMouseLbUpDown(POINT aCurPos, TBool aDown);
void HandleMouseMove(POINT aCurPos);
void ShowMenu(HWND hWnd, POINT aCurPos);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
 	MSG msg;
  // Initialize global strings    
	lstrcpy (szAppName, APPNAME);
	lstrcpy (szTitle, APPTITLE);
	
	if (!hPrevInstance)
	{
		// Perform instance initialization      
		if (!InitApplication(hInstance)) 
		{          
			return (FALSE);       
		}    
	}     
	// Perform application initialization   
	if (!InitInstance(hInstance, nCmdShow)) 
	{       
		return (FALSE);    
	}
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (fape)
		delete fape;
	return 0;
}


BOOL InitApplication(HINSTANCE hInstance) 
{     
	WNDCLASS  wc;     
	memset(&wc, 0, sizeof(wc));
	HWND      hwnd;      
	// Win32 will always set hPrevInstance to NULL, so lets check     
	// things a little closer. This is because we only want a single     
	// version of this app to run at a time     
	hwnd = FindWindow (szAppName, szTitle);     
	if (hwnd) 
	{         
		// We found another version of ourself. Lets defer to it
		if (IsIconic(hwnd)) 
		{             
			ShowWindow(hwnd, SW_RESTORE);         
		}         
		SetForegroundWindow (hwnd);          
		// If this app actually had any functionality, we would         
		// also want to communicate any action that our 'twin'         
		// should now perform based on how the user tried to         
		// execute us.         
		return FALSE;         
	}          
	// Fill in window class structure with parameters that describe         
	// the main window.         
	wc.style         = CS_HREDRAW | CS_VREDRAW;         
	wc.lpfnWndProc   = (WNDPROC)WndProc;         
	wc.cbClsExtra    = 0;         
	wc.cbWndExtra    = 0;         
	wc.hInstance     = hInstance;         
	wc.hIcon         = LoadIcon (hInstance, szAppName);         
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);         
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);          
	wc.lpszMenuName  = szAppName;         
	wc.lpszClassName = szAppName;          
	// Register the window class and return success/failure code.         
	return RegisterClass(&wc);         
} 


//   PURPOSE: Saves instance handle and creates main window 
//        In this function, we save the instance handle in a global variable and 
//        create and display the main program window. 
// 
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) 
{    
	HWND hWnd;     
	hInst = hInstance;
	// Initialize general 
	// Store instance handle in our global variable     
	hWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);     
	if (!hWnd)
	{       
		return (FALSE);    
	}     
	ShowWindow(hWnd, nCmdShow);    
	UpdateWindow(hWnd);     
	// Initiate FAP environment
	fareapainter = new CFT_BArrea_Painter(hWnd);
	fape = CAE_Env::NewL(1, KLogSpecFileName);
	farea = CFT_Area::NewL(KFAreaName, NULL, fareapainter);
	fape->AddL(farea);
	SetTimer(hWnd, KTimerEnv, KTimerEnvElapsInt, NULL);
	return (TRUE); 
} 

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    LRESULT res = 0;
	int wmId, wmEvent;    
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{        
	case WM_COMMAND:          
		wmId    = LOWORD(wParam); 
		// Remember, these are...          
		wmEvent = HIWORD(wParam); // ...different for Win32!           
		//Parse the menu selections:          
		switch (wmId) 
		{ 
//           case IDM_EXIT:                
//				DestroyWindow (hWnd);                
//				break; 
            default:                
				res = DefWindowProc(hWnd, message, wParam, lParam);          
		}          
		break; 
      case WM_PAINT:
          {
			if (fareapainter)
			{
				fareapainter->beginPaint(hWnd);
				farea->Draw();
				fareapainter->endPaint();
			}
		  }
			break;        
	  case WM_DESTROY:          
			// Tell WinHelp we don't need it any more...                
			WinHelp (hWnd, APPNAME".HLP", HELP_QUIT,(DWORD)0);          
			PostQuitMessage(0);          
			break;        
	  case WM_TIMER:
			fape->Step();
			break;        
	  case WM_LBUTTONDOWN:
	  case WM_LBUTTONUP:
		  {
			  // Find Coordinates of mouse cursor.
			  LONG xPos = LOWORD(lParam);
			  LONG yPos = HIWORD(lParam);
			  POINT CurPos = {xPos, yPos};
			  HandleMouseLbUpDown(CurPos, message == WM_LBUTTONDOWN);
		  }
		  break;
	  case WM_MOUSEMOVE:
		  {
			  // Find Coordinates of mouse cursor.
			  LONG xPos = LOWORD(lParam);
			  LONG yPos = HIWORD(lParam);
			  POINT CurPos = {xPos, yPos};
			  HandleMouseMove(CurPos);
		  }
		  break;
	  case WM_RBUTTONUP:
		  {
			// display menu
			LONG xPos = LOWORD(lParam);
			LONG yPos = HIWORD(lParam);
			POINT curPos = {xPos, yPos};
			ShowMenu(hWnd, curPos);
		  }
		  break;
	  default:
          
		  res = DefWindowProc(hWnd, message, wParam, lParam);    
	}    
	return res; 
}

// Handler of mouse left button down press event
void HandleMouseLbUpDown(POINT aCurPos, TBool aDown)
{
	*farea->iLbDown = aDown?1:0;
	*farea->iMcPos = CF_TdPoint(aCurPos.x, aCurPos.y);
}


// Handler of mouse move event
void HandleMouseMove(POINT aCurPos)
{
	*farea->iMcPos = CF_TdPoint(aCurPos.x, aCurPos.y);
}

void ShowMenu(HWND hWnd, POINT aCurPos)
{
	HMENU hMenu = ::CreatePopupMenu();
	if (hMenu != NULL)
	{
		// add a few test items
		::AppendMenu(hMenu, MF_STRING, 1, "Add ball");
		::AppendMenu(hMenu, MF_STRING, 2, "Add with parameters");
		ClientToScreen(hWnd, &aCurPos);
		int sel = ::TrackPopupMenuEx(hMenu, TPM_CENTERALIGN | TPM_RETURNCMD,
					aCurPos.x, aCurPos.y, hWnd, NULL);
		::DestroyMenu(hMenu);
		switch (sel) 
		{ 
			case 1:
				{
				farea->AddBallL(aCurPos.x, aCurPos.y, 300, 30);
				farea->Draw();
				}
				break;
			case 2:
				{
				farea->AddBallL(100, 100, 400, 20);
				farea->Draw();
				}              
				break;
            default:
				break;
		}   
	}

}







