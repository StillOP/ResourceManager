#pragma once
#include <Windows.h>
#include <string>
#include <d2d1.h>

struct ResourceInfo
{
	std::wstring name;
	std::wstring resourceFile;
	D2D1_SIZE_U size;
	UINT bufferSize;
	//UINT compressedBufferSize;
	UINT stride;
	double dpiX;
	double dpiY;
};