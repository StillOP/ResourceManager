#pragma once
//Windows Header Files:
#include <Windows.h>

//C Runtime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
#include <d2d1.h>

//Others include
#include "FileManager.h"
#include "RenderManager.h"


#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do { if(!(b)) {OutputDebugStringA("Assert: " #b "\n"); } } while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	// Register the window class and call methods for instantiating drawing resources
	HRESULT Initialize();

	//Process and dispatch messages
	void RunMessageLoop();

private:
	void CreateMainMenu(HWND hwnd);


	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hwnd;
};

//DXGI_FORMAT_B8G8R8A8_UNORM,