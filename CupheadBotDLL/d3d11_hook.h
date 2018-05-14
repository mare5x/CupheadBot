#pragma once
#include <d3d11.h>

// function pointer typedef for the Present() function in D3D
typedef HRESULT(__stdcall *d3d11_PresentHook)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);


extern IDXGISwapChain* g_p_swapchain;
extern ID3D11Device* g_p_device;
extern ID3D11DeviceContext* g_p_device_context;

/* A dummy window used to create a D3D device and swapchain. */
// Based of hacklib's D3DDeviceFetcher.cpp
struct DummyWindow
{
	DummyWindow()
	{
		WNDCLASSEXA wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_CLASSDC;
		wc.lpfnWndProc = DefWindowProc;
		wc.hInstance = GetModuleHandleA(NULL);
		wc.lpszClassName = "DX";
		RegisterClassExA(&wc);

		m_hWnd =
			CreateWindowA("DX", 0, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), 0, wc.hInstance, 0);
	}

	~DummyWindow()
	{
		if (m_hWnd)
		{
			DestroyWindow(m_hWnd);
			UnregisterClassA("DX", GetModuleHandleA(NULL));
		}
	}

	HWND m_hWnd = NULL;
};


HRESULT __stdcall present_callback(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);


void hook_d3d11();


/** Removes the d3d11 hook on the Present function. 
	NOTE: this works outside present_impl. To unhook from present_impl return true.
*/
void unhook_d3d11();


/** Define your own implementation of the Present() function externally. Called from Present() each frame. 
	Return true if you want to unhook d3d11 cleanly from inside this function.
*/
extern bool present_impl(ID3D11Device* device, ID3D11DeviceContext* device_context, IDXGISwapChain* swap_chain);