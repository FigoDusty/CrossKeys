#include "CrossKeys.h"

#define STR_USAGE "\
This is a enhance tool used to play Civ2 on \
laptops. When presses the combined keys \
such as [Up]+[Left], a diagonal direction key will be \
send to Civ2.\n\n\
[Designed by Figo]"

#define STR_TITLE "CrossKeys v1.0"

DWORD g_KeyInterval = 100;
int gLeftUpKey = VK_NUMPAD7;
int gLeftDownKey = VK_NUMPAD1;
int gRightUpKey = VK_NUMPAD9;
int gRightDownKey = VK_NUMPAD3;

DWORD g_KeyDownTime[256] = {0};

BOOL g_LeftUpKeyPressed = FALSE;
BOOL g_LeftDownKeyPressed = FALSE;
BOOL g_RightUpKeyPressed = FALSE;
BOOL g_RightDownKeyPressed = FALSE;
BOOL g_IsMyKeyDown = FALSE;
HWND g_hWnd;
HHOOK g_KeyboardHook;

NOTIFYICONDATA nid;
HMENU hMenu;
HANDLE hMutex;

int trayIconSize;
HICON iconUL,iconUR,iconDL,iconDR,iconIdle;

void TrayIconUpdate(HICON icon);

//----------------------------------------
// Read settings from the INI file.
// @return TRUE if successful, FALSE otherwise.
//----------------------------------------
BOOL ReadSettingsFromINI(void) {
    char iniFileName[MAX_PATH];
    if (GetModuleFileName(NULL, iniFileName, MAX_PATH) == 0) return FALSE;
    
    char* extPos = strstr(iniFileName, ".exe");
    if (extPos == NULL) return FALSE;
	
	strcpy(extPos, ".ini");
    
    char buffer[256];
    if (GetPrivateProfileString("Settings", "Interval", NULL, buffer, sizeof(buffer), iniFileName))
        g_KeyInterval = atoi(buffer);
    if (GetPrivateProfileString("Key", "LeftUp", NULL, buffer, sizeof(buffer), iniFileName))
        gLeftUpKey = atoi(buffer);
    if (GetPrivateProfileString("Key", "LeftDown", NULL, buffer, sizeof(buffer), iniFileName))
        gLeftDownKey = atoi(buffer);
    if (GetPrivateProfileString("Key", "RightUp", NULL, buffer, sizeof(buffer), iniFileName))
        gRightUpKey = atoi(buffer);
    if (GetPrivateProfileString("Key", "RightDown", NULL, buffer, sizeof(buffer), iniFileName))
        gRightDownKey = atoi(buffer);
    return TRUE;
}

//----------------------------------------
// Get the tick interval since the key was pressed.
// @param vkCode The virtual key code.
// @return The tick interval or 0xffffffff if the key was not pressed.
//----------------------------------------
DWORD GetTickInterval(int vkCode) {
    if (g_KeyDownTime[vkCode] == 0) return 0xffffffff;
    return GetTickCount() - g_KeyDownTime[vkCode];
}

//----------------------------------------
// Check if a combination of two keys is pressed within the specified interval.
// @param vk1 The first virtual key code.
// @param vk2 The second virtual key code.
// @return TRUE if the combination is pressed, FALSE otherwise.
//----------------------------------------
BOOL IsComboKeyDown(int vk1, int vk2) {
    if (g_KeyDownTime[vk1] && GetTickInterval(vk2) <= g_KeyInterval) return 1;
    if (g_KeyDownTime[vk2] && GetTickInterval(vk1) <= g_KeyInterval) return 1;
    return 0;
}

//----------------------------------------
// Timer callback function for key down event.
// @param idEvent The timer identifier, also it is vkCode
//----------------------------------------
VOID CALLBACK onKeyDownTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	KillTimer(hwnd, idEvent);
	g_IsMyKeyDown=1;
	keybd_event(idEvent, 0, 0, 0);
}

//----------------------------------------
// Handle the HOOK_KEYDOWN message.
// @param vkCode The virtual key code.
// @return 1 if the key event is handled, 0 otherwise.
//----------------------------------------
int on_HOOK_KEYDOWN(int vkCode) {
    if (IsComboKeyDown(VK_LEFT, VK_UP)) {
    	KillTimer(g_hWnd, VK_LEFT);
    	KillTimer(g_hWnd, VK_UP);
        keybd_event(gLeftUpKey, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        g_LeftUpKeyPressed = TRUE;
        TrayIconUpdate(iconUL);
        return 1;
    } else if (IsComboKeyDown(VK_LEFT, VK_DOWN)) {
    	KillTimer(g_hWnd, VK_LEFT);
    	KillTimer(g_hWnd, VK_DOWN);
        keybd_event(gLeftDownKey, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        g_LeftDownKeyPressed = TRUE;
        TrayIconUpdate(iconDL);
        return 1;
    } else if (IsComboKeyDown(VK_RIGHT, VK_UP)) {
    	KillTimer(g_hWnd, VK_RIGHT);
    	KillTimer(g_hWnd, VK_UP);
        keybd_event(gRightUpKey, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        g_RightUpKeyPressed = TRUE;
        TrayIconUpdate(iconUR);
        return 1;
    } else if (IsComboKeyDown(VK_RIGHT, VK_DOWN)) {
    	KillTimer(g_hWnd, VK_RIGHT);
    	KillTimer(g_hWnd, VK_DOWN);
        keybd_event(gRightDownKey, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        g_RightDownKeyPressed = TRUE;
        TrayIconUpdate(iconDR);
        return 1;
    }
    else if(vkCode==VK_UP || vkCode==VK_DOWN || vkCode==VK_LEFT ||vkCode==VK_RIGHT){
		SetTimer(g_hWnd, vkCode, g_KeyInterval, onKeyDownTimerProc);
		return 1;
	}
	
    return 0;
}

//----------------------------------------
// Handle the HOOK_KEYUP message.
// @param vkCode The virtual key code.
// @return 1 if the key event is handled, 0 otherwise.
//----------------------------------------
int on_HOOK_KEYUP(int vkCode) {
    if (vkCode == VK_LEFT) {
        if (g_LeftUpKeyPressed && g_KeyDownTime[VK_UP] == 0) {
            keybd_event(gLeftUpKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            g_LeftUpKeyPressed = FALSE;
        	TrayIconUpdate(iconIdle);
            return 1;
        } else if (g_LeftDownKeyPressed && g_KeyDownTime[VK_DOWN] == 0) {
            keybd_event(gLeftDownKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            g_LeftDownKeyPressed = FALSE;
        	TrayIconUpdate(iconIdle);
            return 1;
        }
    } else if (vkCode == VK_UP) {
        if (g_LeftUpKeyPressed && g_KeyDownTime[VK_LEFT] == 0) {
            keybd_event(gLeftUpKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            g_LeftUpKeyPressed = FALSE;
        	TrayIconUpdate(iconIdle);
            return 1;
        } else if (g_RightUpKeyPressed && g_KeyDownTime[VK_RIGHT] == 0) {
            keybd_event(gRightUpKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            g_RightUpKeyPressed = FALSE;
        	TrayIconUpdate(iconIdle);
            return 1;
        }
    } else if (vkCode == VK_RIGHT) {
        if (g_RightUpKeyPressed && g_KeyDownTime[VK_UP] == 0) {
            keybd_event(gRightUpKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            g_RightUpKeyPressed = FALSE;
        	TrayIconUpdate(iconIdle);
            return 1;
        } else if (g_RightDownKeyPressed && g_KeyDownTime[VK_DOWN] == 0) {
            keybd_event(gRightDownKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            g_RightDownKeyPressed = FALSE;
        	TrayIconUpdate(iconIdle);
            return 1;
        }
    } else if (vkCode == VK_DOWN) {
        if (g_LeftDownKeyPressed && g_KeyDownTime[VK_LEFT] == 0) {
            keybd_event(gLeftDownKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            g_LeftDownKeyPressed = FALSE;
        	TrayIconUpdate(iconIdle);
            return 1;
        } else if (g_RightDownKeyPressed && g_KeyDownTime[VK_RIGHT] == 0) {
            keybd_event(gRightDownKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            g_RightDownKeyPressed = FALSE;
        	TrayIconUpdate(iconIdle);
            return 1;
        }
    }
    
    return 0;
}

//----------------------------------------
// Keyboard hook procedure.
// @param nCode The hook code.
// @param wParam The message parameter.
// @param lParam The message parameter.
// @return The result of the next hook in the chain.
//----------------------------------------
LRESULT CALLBACK KeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKeyStruct = (KBDLLHOOKSTRUCT*)lParam;
        int vkCode = pKeyStruct->vkCode;
        if (wParam == WM_KEYDOWN) {
			if(g_IsMyKeyDown){
				g_IsMyKeyDown=0;
			}
			else{
	            g_KeyDownTime[vkCode] = GetTickCount();
	            if (on_HOOK_KEYDOWN(vkCode)) return 1;
	        }
        } else if (wParam == WM_KEYUP) {
            g_KeyDownTime[vkCode] = 0;
            if (on_HOOK_KEYUP(vkCode)) return 1;
        }
    }
    return CallNextHookEx(g_KeyboardHook, nCode, wParam, lParam);
}

//----------------------------------------
// Create the popup menu.
//----------------------------------------
void PopMenuCreate(void){
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING | MF_GRAYED, 0, STR_TITLE);
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 2, "About");
    AppendMenu(hMenu, MF_STRING, 1, "Exit");
}

//----------------------------------------
// Show the popup menu and return the selected command.
// @return The selected command ID.
//----------------------------------------
int PopMenu(void){
	POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(g_hWnd);
    return TrackPopupMenu(hMenu, TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y-32, 0, g_hWnd, NULL);
}

//----------------------------------------
// Create the system tray icon.
//----------------------------------------
void TrayIconCreate(void){
	memset(&nid, 0, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.dwInfoFlags = NIIF_NONE;//no icon
    nid.hWnd = g_hWnd;
    nid.uID = 1;
    nid.uCallbackMessage = WM_COMMAND;
    nid.hIcon = iconIdle;
    strcpy(nid.szTip, STR_TITLE);
    strcpy(nid.szInfoTitle, "");
    strcpy(nid.szInfo, STR_USAGE);
    nid.uTimeout = 500;
    Shell_NotifyIcon(NIM_ADD, &nid);
}

//----------------------------------------
// Update the system tray icon with the specified icon ID.
// @param iconId The ID of the icon to be used.
//----------------------------------------
void TrayIconUpdate(HICON icon) {
	static HICON icon_now=(HICON)-1;
	if(icon==icon_now) return;
	icon_now=icon;
	
	nid.uFlags = NIF_ICON;
    nid.hIcon = icon;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

//----------------------------------------
// Window procedure for handling window messages.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: 
            if (LOWORD(lParam) == WM_RBUTTONUP){
                switch(PopMenu())
                {
	                case 2:
	                    nid.uTimeout = 1000;
						nid.uFlags = NIF_INFO;
	                    Shell_NotifyIcon(NIM_MODIFY, &nid);
	                    break;
	                case 1:
	                    PostQuitMessage(0);
	                    break;
				}
            }
            break;
        
        case WM_CLOSE:
		    PostQuitMessage(0);
        	break;
        
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//----------------------------------------
// The entry point of the application.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int i;
	
    for(i=0; g_hWnd = FindWindow("CrossKeysClass", NULL) ;i++) {
        PostMessage(g_hWnd, WM_CLOSE, 0, 0);
        Sleep(500);
        if(i>10){
            MessageBox(NULL, "Another instance of CrossKeys is still running. Please try again later.", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
            return -2;
        }
    }
    
	ReadSettingsFromINI();
    
    g_KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyBoardHookProc, hInstance, 0);
    if (g_KeyboardHook == NULL) {
        MessageBox(NULL, "Can't install keyboard process...", STR_TITLE, MB_OK | MB_ICONERROR | MB_TOPMOST);
        return -1;
    }
    
    trayIconSize=32;
    
	iconUL = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_UL), IMAGE_ICON, trayIconSize, trayIconSize, LR_DEFAULTCOLOR);
	iconUR = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_UR), IMAGE_ICON, trayIconSize, trayIconSize, LR_DEFAULTCOLOR);
	iconDL = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_DL), IMAGE_ICON, trayIconSize, trayIconSize, LR_DEFAULTCOLOR);
	iconDR = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_DR), IMAGE_ICON, trayIconSize, trayIconSize, LR_DEFAULTCOLOR);
	iconIdle = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_IDLE), IMAGE_ICON, trayIconSize, trayIconSize, LR_DEFAULTCOLOR);
    
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CrossKeysClass";
    RegisterClass(&wc);
    g_hWnd = CreateWindow(wc.lpszClassName, "CrossKeys", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    
    TrayIconCreate();
    PopMenuCreate();
    
	MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
	Shell_NotifyIcon(NIM_DELETE, &nid);
    UnhookWindowsHookEx(g_KeyboardHook);
    CloseHandle(hMutex);
    return msg.wParam;
}
