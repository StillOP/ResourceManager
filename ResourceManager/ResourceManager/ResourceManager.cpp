#include "stdafx.h"
#include "ResourceManager.h"
#include "Utilities.h"

#define IDM_FILE	1
#define IDM_EDIT	2
#define IDM_VIEW	3
#define IDM_NEW		4
#define IDM_OPEN	5
#define IDM_ADD		6
#define IDM_CMPRS	7
#define IDM_DPL		8



ResourceManager::ResourceManager() :
	m_hwnd(NULL)
{}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

HRESULT ResourceManager::Initialize()
{
	HRESULT hr = S_OK;

	//SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	if (SUCCEEDED(hr))
	{
		//Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = ResourceManager::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"ResourceManager";

		RegisterClassEx(&wcex);

		//Create the window.
		m_hwnd = CreateWindow(
			L"ResourceManager",
			L"Colorless Resource Manager",
			WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			600.0f,
			400.0f,
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
		);

		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			ResourceManager app;

			if (SUCCEEDED(app.Initialize()))
			{
				app.RunMessageLoop();
			}
		}
		CoUninitialize();
	}

	return 0;
}


LRESULT CALLBACK ResourceManager::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	FileManager::getFileManager().SetWindowHandle(hwnd);
	RenderManager::getRenderManager().SetWindowHandle(hwnd);

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		ResourceManager* pResourceManager = (ResourceManager *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pResourceManager)
		);
		
		pResourceManager->CreateMainMenu(hwnd);

		result = 1;
	}
	else
	{
		ResourceManager* pResourceManager = reinterpret_cast<ResourceManager*>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA
			)));

		bool wasHandled = false;

		if (pResourceManager)
		{
			switch (message)
			{
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				RenderManager::getRenderManager().Resize(width, height);
			}
			result = 0;
			wasHandled = true;
			break;
			case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, NULL, FALSE);
			}
			result = 0;
			wasHandled = true;
			break;

			case WM_PAINT:
			{
				RenderManager::getRenderManager().Render();
				ValidateRect(hwnd, NULL);
			}
			result = 0;
			wasHandled = true;
			break;

			case WM_COMMAND:
			{
				std::wstring str = L"Menu : " + std::to_wstring(LOWORD(wParam));
				OutputDebugString(str.c_str());

				switch (LOWORD(wParam))
				{
				case IDM_NEW:
				{
					RenderManager::getRenderManager().ClearBitmaps();
					FileManager::getFileManager().NewFile();
				}
				break;
				case IDM_OPEN:
				{
					RenderManager::getRenderManager().ClearBitmaps();
					FileManager::getFileManager().OpenFile();
				}
				break;
				case IDM_ADD:
				{
					FileManager::getFileManager().AddBitmap();
				}
				break;
				case IDM_CMPRS:
				{
					FileManager::getFileManager().CompressResourceData();
				}
				break;
				case IDM_DPL:
				{
					FileManager::getFileManager().DecompressResourceData(
						RenderManager::getRenderManager().GetRenderTarget(), 
						RenderManager::getRenderManager().GetBitmapContaier());

					InvalidateRect(hwnd, NULL, FALSE);
				}
				break;
				}

				result = 0;
				wasHandled = true;
				break;
			}
			result = 0;
			wasHandled = true;
			break;

			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			result = 1;
			wasHandled = true;
			break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}

void ResourceManager::CreateMainMenu(HWND hwnd)
{
	HMENU hmenu = CreateMenu();
	HMENU hfileMenu = CreatePopupMenu();
	HMENU heditMenu = CreatePopupMenu();
	HMENU hviewMenu = CreatePopupMenu();
	
	MENUITEMINFO submenuItemInfo = { sizeof(MENUITEMINFO) };
	submenuItemInfo.fMask = MIIM_STRING | MIIM_ID;
	submenuItemInfo.fType = MFT_STRING;
	submenuItemInfo.fState = MFS_ENABLED;

	submenuItemInfo.dwTypeData = (LPTSTR)L"New";
	submenuItemInfo.cch = sizeof(L"New");
	submenuItemInfo.wID = IDM_NEW;
	InsertMenuItem(hfileMenu, (UINT)IDM_NEW, FALSE, &submenuItemInfo);

	submenuItemInfo.dwTypeData = (LPTSTR)L"Open";
	submenuItemInfo.cch = sizeof(L"Open");
	submenuItemInfo.wID = IDM_OPEN;
	InsertMenuItem(hfileMenu, (UINT)IDM_OPEN, FALSE, &submenuItemInfo);

	submenuItemInfo.dwTypeData = (LPTSTR)L"Compress";
	submenuItemInfo.cch = sizeof(L"Compress");
	submenuItemInfo.wID = IDM_CMPRS;
	InsertMenuItem(hfileMenu, (UINT)IDM_CMPRS, FALSE, &submenuItemInfo);

	submenuItemInfo.dwTypeData = (LPTSTR)L"Add";
	submenuItemInfo.cch = sizeof(L"Add");
	submenuItemInfo.wID = IDM_ADD;
	InsertMenuItem(heditMenu, (UINT)IDM_ADD, FALSE, &submenuItemInfo);

	submenuItemInfo.dwTypeData = (LPTSTR)L"Display bitmaps";
	submenuItemInfo.cch = sizeof(L"Display bitmpas");
	submenuItemInfo.wID = IDM_DPL;
	InsertMenuItem(hviewMenu, (UINT)IDM_DPL, FALSE, &submenuItemInfo);

	MENUITEMINFO menuItemInfo = { sizeof(MENUITEMINFO) };
	menuItemInfo.fMask = MIIM_STRING | MIIM_SUBMENU;
	menuItemInfo.fType = MFT_STRING;
	menuItemInfo.fState = MFS_ENABLED;


	menuItemInfo.dwTypeData = (LPTSTR)L"File";
	menuItemInfo.cch = sizeof(L"File");
	menuItemInfo.hSubMenu = hfileMenu;
	InsertMenuItem(hmenu, (UINT)IDM_FILE, FALSE, &menuItemInfo);

	menuItemInfo.dwTypeData = (LPTSTR)L"Edit";
	menuItemInfo.cch = sizeof(L"Edit");
	menuItemInfo.hSubMenu = heditMenu;
	InsertMenuItem(hmenu, (UINT)IDM_EDIT, FALSE, &menuItemInfo);

	menuItemInfo.dwTypeData = (LPTSTR)L"View";
	menuItemInfo.cch = sizeof(L"View");
	menuItemInfo.hSubMenu = hviewMenu;
	InsertMenuItem(hmenu, (UINT)IDM_VIEW, FALSE, &menuItemInfo);

	SetMenu(hwnd, hmenu);
}