#pragma once
#include <Windows.h>
#include <commdlg.h>
#include <vector>

#include "Utilities.h"
#include "ResourceInfo.h"

using BitmapContainer = std::vector<ID2D1Bitmap*>;

class FileManager
{
public:
	FileManager();
	~FileManager();

	BOOL NewFile();
	BOOL OpenFile();

	void AddBitmap();
	void CompressResourceData();
	void DecompressResourceData(ID2D1HwndRenderTarget* renderTarget, BitmapContainer& bitmapContainer);

	void SetWindowHandle(HWND hwnd);

	static FileManager& getFileManager()
	{
		static FileManager sfileManager;
		return sfileManager;
	}

private:
	HRESULT RegisterBitmapData(LPWSTR uri, LPWSTR id);

	void WriteBitmapData(BYTE* bitmapData, DWORD bitmapDataSize);
	void RegisterResource(ResourceInfo& info);

	void ClearRegisterResource();

	HWND m_hwnd;

	std::wstring m_currentWorkingFilePath;
	std::wstring m_currentWorkingFileTitle;

	std::vector<ResourceInfo> m_resourcesInfo;
};