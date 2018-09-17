#include "stdafx.h"
#include "FileManager.h"
#include <string>
#include <sstream>


FileManager::FileManager()
	:m_hwnd{ NULL }
{}

FileManager::~FileManager()
{
}

BOOL FileManager::NewFile()
{
	ClearRegisterResource();

	WCHAR filepathbuffer[260];
	WCHAR filenamebuffer[100];

	OPENFILENAME ofna = { sizeof(ofna) };
	ofna.hwndOwner = m_hwnd;
	ofna.lpstrFilter = NULL;
	ofna.lpstrCustomFilter = NULL;
	ofna.lpstrFile = filepathbuffer;
	ofna.lpstrFile[0] = '\0';
	ofna.nMaxFile = sizeof(filepathbuffer);
	ofna.lpstrFileTitle = filenamebuffer;
	ofna.nMaxFileTitle = sizeof(filenamebuffer);

	if (GetSaveFileName(&ofna) == TRUE)
	{
		m_currentWorkingFilePath = ofna.lpstrFile;
		m_currentWorkingFileTitle = ofna.lpstrFileTitle;

		return TRUE;
	}

	return FALSE;
}

 BOOL FileManager::OpenFile()
 {
	 ClearRegisterResource();

	 WCHAR filepathbuffer[260];
	 WCHAR filenamebuffer[100];

	 OPENFILENAME ofna = { sizeof(ofna) };
	 ofna.hwndOwner = m_hwnd;
	 ofna.lpstrFilter = NULL;
	 ofna.lpstrCustomFilter = NULL;
	 ofna.lpstrFile = filepathbuffer;
	 ofna.lpstrFile[0] = '\0';
	 ofna.nMaxFile = sizeof(filepathbuffer);
	 ofna.lpstrFileTitle = filenamebuffer;
	 ofna.nMaxFileTitle = sizeof(filenamebuffer);
	 ofna.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	 if (GetOpenFileName(&ofna) == TRUE)
	 {
		 m_currentWorkingFilePath = ofna.lpstrFile;
		 m_currentWorkingFileTitle = ofna.lpstrFileTitle;
	 }

	 return TRUE;
 }

void FileManager::AddBitmap()
{
	WCHAR filepathbuffer[260];
	WCHAR filenamebuffer[260];

	OPENFILENAME ofna = { sizeof(ofna) };
	ofna.hwndOwner = m_hwnd;
	ofna.lpstrFilter = NULL;
	ofna.lpstrCustomFilter = NULL;
	ofna.lpstrFile = filepathbuffer;
	ofna.lpstrFile[0] = '\0';
	ofna.nMaxFile = sizeof(filepathbuffer);
	ofna.lpstrFileTitle = filenamebuffer;
	ofna.nMaxFileTitle = sizeof(filenamebuffer);
	ofna.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;



	if (GetOpenFileName(&ofna) == TRUE)
	{
		RegisterBitmapData(ofna.lpstrFile, ofna.lpstrFileTitle);
	}
}

void FileManager::WriteBitmapData(BYTE* bitmapData, DWORD bitmapDataSize)
{
	HANDLE huncompressedFile = INVALID_HANDLE_VALUE;

	DWORD bytesWritten;
	BOOL success;

	std::wstring uncompressedFileName = L"C:\\Resource Manager\\UC_" + m_currentWorkingFileTitle;

	huncompressedFile = CreateFile(
		uncompressedFileName.c_str(),
		FILE_APPEND_DATA,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (huncompressedFile == INVALID_HANDLE_VALUE)
	{
		std::wstring str = L"Unable to create or the open the uncompressed version!";
		OutputDebugString(str.c_str());
		return;
	}

	success = WriteFile(huncompressedFile, bitmapData, bitmapDataSize, &bytesWritten, NULL);

	if ((bytesWritten != bitmapDataSize || !success))
	{
		std::wstring str = L"Unable to write compressed data to file!";
		OutputDebugString(str.c_str());

		CloseHandle(huncompressedFile);
		return;
	}

	success = WriteFile(huncompressedFile, "\r\n", 2, &bytesWritten, NULL);

	if ((bytesWritten != 2 || !success))
	{
		std::wstring str = L"Unable to add the new line character!";
		OutputDebugString(str.c_str());

		CloseHandle(huncompressedFile);
		return;
	}

	CloseHandle(huncompressedFile);
}

HRESULT FileManager::RegisterBitmapData(LPWSTR uri, LPWSTR id)
{
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICBitmap* pBitmap = NULL;
	IWICBitmapLock* pLock = NULL;

	IWICImagingFactory* m_pIWICFactory = NULL;

	ResourceInfo info;

	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pIWICFactory)
	);


	if (SUCCEEDED(hr))
	{
		hr = m_pIWICFactory->CreateDecoderFromFilename(
			uri,
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&pDecoder
		);


		if (SUCCEEDED(hr))
		{
			hr = pDecoder->GetFrame(0, &pSource);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_pIWICFactory->CreateBitmapFromSource(
				pSource,
				WICBitmapCacheOnDemand,
				&pBitmap
			);
		}
		if (SUCCEEDED(hr))
		{
			UINT lockWidth, lockHeight;
			pBitmap->GetSize(&lockWidth, &lockHeight);

			WICRect rcLock = { 0, 0, lockWidth, lockHeight };

			hr = pBitmap->Lock(&rcLock, WICBitmapLockRead, &pLock);
		}
	}
	if (SUCCEEDED(hr))
	{
		BYTE* source = NULL;
		UINT cbBufferSize = 0;
		UINT stride = 0;

		hr = pLock->GetDataPointer(&cbBufferSize, &source);
		pLock->GetStride(&stride);

		if (SUCCEEDED(hr))
		{
			D2D1_SIZE_U size;
			pBitmap->GetSize(&size.width, &size.height);

			double dpiX, dpiY;
			hr = pBitmap->GetResolution(&dpiX, &dpiY);

			if (SUCCEEDED(hr))
			{
				info.name = (std::wstring) id;
				info.resourceFile = (std::wstring)m_currentWorkingFileTitle;
				info.bufferSize = cbBufferSize;
				info.size = size;
				info.dpiX = dpiX;
				info.dpiY = dpiY;
				info.stride = stride;

				m_resourcesInfo.push_back(info);

				OutputDebugString((LPWSTR)source);
				OutputDebugStringW(L"\n");

				WriteBitmapData(source, (DWORD)info.bufferSize);
				RegisterResource(info);
			}
		}
	}

	SafeRelease(&pLock);
	SafeRelease(&pBitmap);
	SafeRelease(&pDecoder);
	SafeRelease(&pSource);

	return hr;
}

void FileManager::CompressResourceData()
{
	COMPRESSOR_HANDLE compressor = NULL;

	HANDLE huncompressedFile = INVALID_HANDLE_VALUE;
	HANDLE hcompressedFile = INVALID_HANDLE_VALUE;

	PBYTE uncompressedBuffer = NULL;
	PBYTE compressedBuffer = NULL;

	SIZE_T compressedDataSize, compressedBufferSize;
	DWORD uncompressedFileSize, bytesRead, bytesWritten;

	LARGE_INTEGER fileSize;

	BOOL success;

	ULONGLONG startTime, endTime;
	double timeDuration;

	auto lReleaseResources = [&]() {
		if (huncompressedFile != INVALID_HANDLE_VALUE) { CloseHandle(huncompressedFile); }
		if (hcompressedFile != INVALID_HANDLE_VALUE) { CloseHandle(hcompressedFile); }
		if (compressor != NULL) { CloseCompressor(compressor); }
		if (uncompressedBuffer) { delete[] uncompressedBuffer; uncompressedBuffer = NULL; }
		if (compressedBuffer) { delete[] compressedBuffer; compressedBuffer = NULL; }
	};

	std::wstring uncompressedFileName = L"C:\\Resource Manager\\UC_" + m_currentWorkingFileTitle;

	huncompressedFile = CreateFile(
		uncompressedFileName.c_str(),
		GENERIC_READ | DELETE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (huncompressedFile == INVALID_HANDLE_VALUE)
	{
		std::wstring str = L"Unable to open the uncompressed file version!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	success = GetFileSizeEx(huncompressedFile, &fileSize);
	if (!success || fileSize.QuadPart > 0xFFFFFFFF)
	{
		std::wstring str = L"Unable to get the uncompressed file version size!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	uncompressedFileSize = fileSize.LowPart;

	uncompressedBuffer = new BYTE[uncompressedFileSize];
	ReadFile(huncompressedFile, uncompressedBuffer, uncompressedFileSize, &bytesRead, NULL);

	if (!success || bytesRead != uncompressedFileSize)
	{
		std::wstring str = L"Unable to read in the uncompressed file version!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	success = CreateCompressor(
		COMPRESS_ALGORITHM_LZMS,
		NULL,
		&compressor
	);

	if (!success)
	{
		std::wstring str = L"Unable to create a compressor!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	success = Compress(
		compressor,
		uncompressedBuffer,
		uncompressedFileSize,
		NULL,
		0,
		&compressedBufferSize);

	if (!success)
	{
		DWORD errorCode = GetLastError();

		if (errorCode != ERROR_INSUFFICIENT_BUFFER)
		{
			std::wstring str = L"Unable to compress data!";
			OutputDebugString(str.c_str());

			lReleaseResources();
			return;
		}
	
		compressedBuffer = new BYTE[compressedBufferSize];
		if (!compressedBuffer)
		{
			std::wstring str = L"Unable to allocate memeory for compressed buffer!";
			OutputDebugString(str.c_str());

			lReleaseResources();
			return;
		}
	}

	startTime = GetTickCount64();

	success = Compress(
		compressor,
		uncompressedBuffer,
		uncompressedFileSize,
		compressedBuffer,
		compressedBufferSize,
		&compressedDataSize
	);

	OutputDebugString(L"\n");
	std::wstring str1 = L"Uncompressed data size : " + std::to_wstring(uncompressedFileSize);
	OutputDebugString(str1.c_str());

	OutputDebugString(L"\n");
	std::wstring str2 = L"Compressed data size : " + std::to_wstring(compressedDataSize);
	OutputDebugString(str2.c_str());

	if (!success)
	{
		std::wstring str = L"Unable to compress data!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	endTime = GetTickCount64();
	timeDuration = (endTime - startTime) / 1000.0;

	hcompressedFile = CreateFile(
		m_currentWorkingFilePath.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (huncompressedFile == INVALID_HANDLE_VALUE)
	{
		std::wstring str = L"Unable to create the compressed file version!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	success = WriteFile(
		hcompressedFile,
		compressedBuffer,
		compressedDataSize,
		&bytesWritten,
		NULL);

	if ((bytesWritten != compressedDataSize || !success))
	{
		std::wstring str = L"Unable to write compressed data to file!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	std::wstring str = L"File compressed in " + std::to_wstring(timeDuration);
	OutputDebugString(str.c_str());

	lReleaseResources();

	success = DeleteFile(uncompressedFileName.c_str());

	if (!success)
	{
		std::wstring str = L"Unable to delete the uncompressed file version!";
		OutputDebugString(str.c_str());
	}
}

void FileManager::RegisterResource(ResourceInfo& info)
{
	//name | resource file name | buffer size | compressed buffer size | stride | width | height | dpiX | dpiY

	HANDLE resourceFileDataBase = INVALID_HANDLE_VALUE;

	OutputDebugString(L"\n");
	OutputDebugString(L"ADDING DATA TO RESOURCE FILE!");
	OutputDebugString(L"\n");

	resourceFileDataBase = CreateFile(
		L"C:\\Resource Manager\\resource.rdb",
		FILE_APPEND_DATA,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DWORD bytesWritten;

	std::wstring inputString = L"#" + info.name + L" " + info.resourceFile + L" "
		+ std::to_wstring(info.bufferSize) + L" " + std::to_wstring(info.stride) + L" "
		+ std::to_wstring(info.size.width) + L" " + std::to_wstring(info.size.height) + L" "
		+ std::to_wstring(info.dpiX) + L" " + std::to_wstring(info.dpiY) + L"\r\n";

	OutputDebugString(L"\n");
	OutputDebugString(L"resource DB info : ");
	OutputDebugString(inputString.data());

	int lenght = WideCharToMultiByte(CP_UTF8, 0, inputString.c_str(), lstrlenW(inputString.c_str()), NULL, 0, NULL, NULL);
	char* convertedInputString = new char[lenght];
	WideCharToMultiByte(CP_UTF8, 0, inputString.c_str(), lstrlenW(inputString.c_str()), convertedInputString, lenght, NULL, NULL);

	BOOL success = WriteFile(
		resourceFileDataBase,
		convertedInputString,
		lenght * sizeof(char),
		&bytesWritten,
		NULL);

	delete[] convertedInputString;
	convertedInputString = NULL;

	OutputDebugString(L"\n");
	OutputDebugString(L"Write file success : ");
	OutputDebugString(std::to_wstring(success).c_str());

	CloseHandle(resourceFileDataBase);
}

void FileManager::DecompressResourceData(ID2D1HwndRenderTarget* renderTarget, BitmapContainer& bitmapContainer)
{
	DECOMPRESSOR_HANDLE decompressor = NULL;

	HANDLE hcompressedFile = INVALID_HANDLE_VALUE;

	PBYTE compressedBuffer = NULL;
	PBYTE decompressedBuffer = NULL;

	BOOL success;
	SIZE_T decompressedBufferSize, decompressedDataSize;
	DWORD compressedFileSize, bytesRead, bytesWriten;
	ULONGLONG startTime, endTime;
	LARGE_INTEGER fileSize;
	double timeDuration;

	auto lReleaseResources = [&]() {
		if (decompressor != NULL) { CloseDecompressor(decompressor); }
		if (compressedBuffer) { delete[] compressedBuffer; compressedBuffer = NULL; }
		if (decompressedBuffer) { delete[] decompressedBuffer; decompressedBuffer = NULL; }
		if (hcompressedFile != INVALID_HANDLE_VALUE) { CloseHandle(hcompressedFile); }
	};

	OutputDebugString(L"\n");
	OutputDebugString(L"Current File decompressed : ");
	OutputDebugString(m_currentWorkingFilePath.c_str());

	hcompressedFile= CreateFile(
		m_currentWorkingFilePath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hcompressedFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(L"\n");
		OutputDebugString(L"Unable to open the working file!");
		OutputDebugString(L"\n");
		OutputDebugString(m_currentWorkingFilePath.c_str());

		lReleaseResources();
		return;
	}

	success = GetFileSizeEx(hcompressedFile, &fileSize);
	if (!success || fileSize.QuadPart > 0xFFFFFFFF)
	{
		std::wstring str = L"Unable to get the compressed file size!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	compressedFileSize = fileSize.LowPart;

	compressedBuffer = new BYTE[compressedFileSize];
	if (!compressedBuffer)
	{
		std::wstring str = L"Unable to allocate memory for the compressed buffer!";
		OutputDebugString(str.c_str());
		return;
	}

	success = ReadFile(hcompressedFile, compressedBuffer, compressedFileSize, &bytesRead, NULL);
	if (!success || bytesRead != compressedFileSize)
	{
		std::wstring str = L"Unable to read in the compressed file!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	success = CreateDecompressor(
		COMPRESS_ALGORITHM_LZMS,
		NULL,
		&decompressor);

	if (!success)
	{
		std::wstring str = L"Unable to create a decompressor!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}

	success = Decompress(
		decompressor,
		compressedBuffer,
		compressedFileSize,
		NULL,
		0,
		&decompressedBufferSize);

	if (!success)
	{
		DWORD errorCode = GetLastError();

		if (errorCode != ERROR_INSUFFICIENT_BUFFER)
		{
			std::wstring str = L"Unable to decompress!";
			OutputDebugString(str.c_str());

			lReleaseResources();
			return;
		}

		decompressedBuffer = new BYTE[decompressedBufferSize];
		if (!decompressedBuffer)
		{
			std::wstring str = L"Unable to allocate memory for the decompressed file!";
			OutputDebugString(str.c_str());

			lReleaseResources();
			return;
		}
	}

	startTime = GetTickCount64();

	success = Decompress(
		decompressor,
		compressedBuffer,
		compressedFileSize,
		decompressedBuffer,
		decompressedBufferSize,
		&decompressedDataSize);

	if (!success)
	{
		std::wstring str = L"Unable to decompress the data!";
		OutputDebugString(str.c_str());

		lReleaseResources();
		return;
	}
	OutputDebugString(L"\n");
	OutputDebugString(L"Compressed data size :");
	OutputDebugString(L"\n");
	OutputDebugString(std::to_wstring(compressedFileSize).c_str());

	OutputDebugString(L"\n");
	OutputDebugString(L"Decompressed data size :");
	OutputDebugString(L"\n");
	OutputDebugString(std::to_wstring(decompressedDataSize).c_str());

	endTime = GetTickCount64();

	timeDuration = (endTime - startTime) / 10000.0;

	std::wstring str = L"Decompression succeed! in " + std::to_wstring(timeDuration);
	OutputDebugString(str.c_str());

	D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);

	std::stringstream decompressedBufferStream((char*)decompressedBuffer);
	std::string line;

	auto iinfo = m_resourcesInfo.begin();

	while (std::getline(decompressedBufferStream, line))
	{
		OutputDebugString(L"\n");
		OutputDebugString(L"LINE : ");
		OutputDebugString(L"\n");
		OutputDebugString((LPWSTR)line.c_str());
		OutputDebugString(L"\n");

		if (iinfo == m_resourcesInfo.end()) { break; }

		ID2D1Bitmap* pBitmap = NULL;

		D2D1_BITMAP_PROPERTIES properties = D2D1::BitmapProperties();
		properties.pixelFormat = pixelFormat;
		properties.dpiX = iinfo->dpiX;
		properties.dpiY = iinfo->dpiY;

		HRESULT hr = renderTarget->CreateBitmap(iinfo->size, (PBYTE)line.c_str(), iinfo->stride, &properties, &pBitmap);

		if (SUCCEEDED(hr))
		{
			bitmapContainer.push_back(pBitmap);
		}
		else
		{
			OutputDebugString(L"\n");
			OutputDebugString(L"Unable to create bitmap : ");
			OutputDebugString(L"\n");
		}

		iinfo++;
	}
	
	OutputDebugString(L"\n");
	OutputDebugString(L"BITMAP CONTAINER SIZE : ");
	OutputDebugString(L"\n");
	OutputDebugString(std::to_wstring(bitmapContainer.size()).c_str());
	OutputDebugString(L"\n");

	lReleaseResources();
}

void FileManager::ClearRegisterResource()
{
	if (!m_resourcesInfo.empty())
	{
		m_resourcesInfo.clear();
	}
}


void FileManager::SetWindowHandle(HWND hwnd) 
{ 
	if (!m_hwnd)
	{
		m_hwnd = hwnd;
	}
}