#pragma once
#include <Windows.h>
#include <d2d1.h>
#include <vector>

#include "ResourceInfo.h"

class RenderManager
{
public:
	RenderManager();
	~RenderManager();

	HRESULT Render();
	void Resize(UINT width, UINT height);

	HRESULT CreateDeviceResources();
	void DiscardDeviceResources();

	void ClearBitmaps();

	void SetWindowHandle(HWND hwnd);

	static RenderManager& getRenderManager()
	{
		static RenderManager srenderManager;
		return srenderManager;
	}

	ID2D1HwndRenderTarget* GetRenderTarget();
	std::vector<ID2D1Bitmap*>& GetBitmapContaier();


private:
	HWND m_hwnd;

	ID2D1Factory* m_pFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	std::vector<ID2D1Bitmap*> m_vBitmaps;
};