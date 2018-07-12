#include <windows.h>
#include "rc95txtedit.h"

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		
		case WM_CREATE:
		{
			HMENU hMenu;
			HMENU hSubMenu;
			HICON hIcon, hIconSm;
			HFONT hfDefault;
			HWND hEdit;
			hMenu = CreateMenu();
			hSubMenu = CreatePopupMenu();
			AppendMenu(hSubMenu, MF_STRING, ID_FILE_OPEN,"Open");
			AppendMenu(hSubMenu, MF_STRING, ID_FILE_SAVE,"Save");
			AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT,"E&xit");									
			AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");
			
			hSubMenu = CreatePopupMenu();
			AppendMenu(hSubMenu, MF_STRING, ID_HELP_ABOUT, "&About");
			AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Help");
									
			SetMenu(hwnd, hMenu);
			
			hIcon = LoadImage(GetModuleHandle(None),
			TXTEDIT_ICONNAME,
			IMAGE_ICON,
			32,
			32,
			LR_LOADFROMFILE);
			if (hIcon)
				SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			else
				MessageBox(hwnd, "Unable to load the large icon.","Error", MB_OK | MB_ICONERROR);
			hIconSm = LoadImage(GetModuleHandle(None),
			TXTEDIT_ICONNAME,
			IMAGE_ICON,
			16,
			16,
			LR_LOADFROMFILE);
			if (hIconSm)
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);
			else
				MessageBox(hwnd, "Unable to load the large icon.","Error", MB_OK | MB_ICONERROR);				
			hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, 
				"EDIT", 
				"",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				0,
				0,
				TXTEDIT_WIDTH,
				TXTEDIT_HEIGHT, 
				hwnd, 
				(HMENU)IDC_MAIN_EDIT, 
				GetModuleHandle(None), 
				None);
			if ( hEdit==None )
				MessageBox(hwnd, "Failed to create a text-editing box.", "Window Error", MB_OK | MB_ICONERROR);
			hfDefault = GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
			break;
		}
		case WM_SIZE:
		{
			HWND hEdit;
			RECT rcClient;
			
			GetClientRect(hwnd, &rcClient);
			hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit, None, 0,0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case ID_FILE_EXIT:
				{
					PostMessage(hwnd, WM_CLOSE, 0,0);
					break;
				}
				case ID_FILE_OPEN:
				{
					CanOperateOpenSaveFile(hwnd, FALSE);
					break;
				}
				case ID_FILE_SAVE:
				{
					CanOperateOpenSaveFile(hwnd, TRUE);
					break;
				}
				case ID_HELP_ABOUT:
				{
					MessageBox(hwnd,"RC95 Text Editor\nVersion 4.0 (Build 1381)\nThe Chehot's Republic of CATV 95\nCopyright © 1997 - 2019\nAll rights reserved.","About RC95 Text Editor", 0);
					break;
				}
			}
			break;
		}
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

BOOL LoadTextFileToEdit(HWND hEdit, LPCTSTR pszFileName)
{
	HANDLE hFile;
	BOOL bSuccess = FALSE;
	
	hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, None, OPEN_EXISTING, 0, None);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize;
		
		dwFileSize = GetFileSize(hFile,None);
		if ( dwFileSize != 0xFFFFFFFF )
		{
			LPSTR pszFileText;
			
			pszFileText = GlobalAlloc(GPTR, dwFileSize + 1);
			if (pszFileText!=None)
			{
				DWORD dwRead;
				if (ReadFile(hFile,pszFileText,dwFileSize,&dwRead,None))
				{
					pszFileText[dwFileSize] = 0;
					if (SetWindowText(hEdit,pszFileText))
						bSuccess=TRUE;
				}
				GlobalFree(pszFileText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

BOOL SaveTextFileFromEdit(HWND hEdit, LPCTSTR pszFileName)
{
	HANDLE hFile;
	BOOL bSuccess = FALSE;
	
	hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, None,
	CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, None);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD dwTextLength;
		
		dwTextLength = GetWindowTextLength(hEdit);
		if ( dwTextLength > 0 )
		{
			LPSTR pszText;
			DWORD dwBufferSize = dwTextLength + 1;
			
			pszText = GlobalAlloc(GPTR, dwBufferSize);
			if (pszText != None)
			{
				if (GetWindowText(hEdit, pszText, dwBufferSize))
				{
					DWORD dwWritten;
					
					if ( WriteFile(hFile, pszText, dwTextLength, &dwWritten, None) )
						bSuccess = TRUE;
				}
				GlobalFree(pszText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

BOOL CanOperateOpenSaveFile(HWND hwnd, BOOL bSaving)
{
	OPENFILENAME ofn;
	BOOL bSuccess=FALSE;
	char szFileName[MAX_PATH] = "";
	
	ZeroMemory(&ofn, sizeof(ofn));
	
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile=MAX_PATH;
	if ( bSaving )
	{
		// add flags
		ofn.Flags = OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
		if ( GetSaveFileName(&ofn) )
		{		
			if ( !SaveTextFileFromEdit(GetDlgItem(hwnd,IDC_MAIN_EDIT), szFileName) )
			{
				MessageBox(hwnd, "File Saving Error!\nUnable to save file.","File Operations Error",MB_OK|MB_ICONSTOP);
				return FALSE;
			}
			bSuccess=TRUE;			
		}
	}
	else
	{
		ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
		if ( GetOpenFileName(&ofn) )
		{
			if ( !LoadTextFileToEdit(GetDlgItem(hwnd, IDC_MAIN_EDIT), szFileName) )
			{
				MessageBox(hwnd, "File Opening Error!\nUnable to open file.","File Operations Error",MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
	}
	return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;

	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(None, IDC_ARROW);
	
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName = "MENUNAME";
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(GetModuleHandle(None), MAKEINTRESOURCE(IDI_ICONS)); /* Load a standard icon */
	wc.hIconSm		 = (HICON)LoadImage(GetModuleHandle(None), MAKEINTRESOURCE(IDI_ICONS), IMAGE_ICON,16,16,0); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(None, "Unable to register Window class!\nPress OK to terminate the program.","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass",TXTEDIT_TITLE,WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		TXTEDIT_WIDTH, /* width */
		TXTEDIT_HEIGHT, /* height */
		None,None,hInstance,None);

	if(hwnd == None) {
		MessageBox(None, "Unable to load Windows application!","Error!",MB_ICONSTOP|MB_OK);
		return 0;
	}

	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, None, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}
