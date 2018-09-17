#include "stdafx.h"
#include "RenderManager.h"
#include "Utilities.h"

#include <fstream>
#include <sstream>

RenderManager::RenderManager()
	: m_pFactory{ NULL }, m_pRenderTarget{ NULL }
{
	D2D1CreateFactory(D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
}

RenderManager::~RenderManager()
{
	SafeRelease(&m_pRenderTarget);

	for (auto& itr : m_vBitmaps)
	{
		SafeRelease(&itr);
	}
}

HRESULT RenderManager::Render()
{
	HRESULT hr = S_OK;
	OutputDebugString(L"\n");
	OutputDebugString(L"BITMAPS SIZE");
	OutputDebugString(L"\n");
	OutputDebugString(std::to_wstring((UINT)m_vBitmaps.size()).c_str());
	OutputDebugString(L"\n");

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		if (!m_vBitmaps.empty())
		{
			OutputDebugString(L"\n");
			OutputDebugString(L"No empty!");
			OutputDebugString(L"\n");
			int i = 0;
			FLOAT offset = 0;
			for (auto itr = m_vBitmaps.begin(); itr != m_vBitmaps.end(); ++itr)
			{
				i++;

				OutputDebugString(L"\n");
				OutputDebugString(std::to_wstring(i).c_str());
				OutputDebugString(L"\n");

				D2D1_SIZE_F size = (*itr)->GetSize();

				offset += itr == m_vBitmaps.begin() ? 0.0f : ((*(itr - 1))->GetSize().height) / 10 + 1.0f;

				D2D1_POINT_2F upperLeftCorner = D2D1::Point2F(0.f, offset);

				// Draw a bitmap.
				m_pRenderTarget->DrawBitmap(
					*itr,
					D2D1::RectF(
						upperLeftCorner.x,
						upperLeftCorner.y,
						upperLeftCorner.x + size.width / 10,
						upperLeftCorner.y + size.height / 10)
				);

			}
		}
		hr = m_pRenderTarget->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DiscardDeviceResources();
	}

	return hr;
}

void RenderManager::ClearBitmaps()
{
	if (!m_vBitmaps.empty())
	{
		for (auto& itr : m_vBitmaps)
		{
			SafeRelease(&itr);
		}
		m_vBitmaps.clear();
	}
}

void RenderManager::Resize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}


HRESULT RenderManager::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);

		//Create a Direct2D render target.
		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_IGNORE
		);

		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
		props.pixelFormat = pixelFormat;


		hr = m_pFactory->CreateHwndRenderTarget(
			props,
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
		);
	}

	return hr;
}

void RenderManager::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
}



void RenderManager::SetWindowHandle(HWND hwnd) 
{
	if (!m_hwnd)
	{
		m_hwnd = hwnd;
	}
}

ID2D1HwndRenderTarget* RenderManager::GetRenderTarget()
{
	return m_pRenderTarget;
}

std::vector<ID2D1Bitmap*>& RenderManager::GetBitmapContaier()
{
	return m_vBitmaps;
}