// Based off:
// https://github.com/Rebzzel/Universal-D3D11-Hook
// https://bitbucket.org/rafzi/hacklib
// https://www.unknowncheats.me/forum/d3d-tutorials-and-source/88369-universal-d3d11-hook.html


#include "memory_tools.h"
#include "d3d11_hook.h"


d3d11_Present d3d11_hook::g_p_present = nullptr;;

d3d11_Present p_post_present_hook = nullptr;
d3d11_present_impl p_present_impl = nullptr;

IDXGISwapChain* d3d11_hook::g_p_swapchain;
ID3D11Device*d3d11_hook:: g_p_device;
ID3D11DeviceContext* d3d11_hook::g_p_device_context;

BYTE* post_detour_cave = nullptr;


HRESULT __stdcall d3d11_hook::present_callback(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	// this function's declaration/signature must be identical to the original Present function's signature
	g_p_swapchain = pSwapChain;
	pSwapChain->GetDevice(__uuidof(g_p_device), (void**)&g_p_device);
	g_p_device->GetImmediateContext(&g_p_device_context);

	// call the user defined implementation 
	if (p_present_impl(g_p_device, g_p_device_context, pSwapChain)) {
		// unhook d3d11 cleanly
		// restore original code and jump directly to the original instead of going through the post detour cave
		// Instead of calling Present() it would be better to jump to it, because now Present returns its value to this function
		// and this function returns the value to the function that called Present initially
		unhook_d3d11();
		return g_p_present(pSwapChain, SyncInterval, Flags);
	}

	// return to the original Present function using the 'original' parameters, by first going to the 
	// code cave which contains the code replaced by the detour hook in Present
	return p_post_present_hook(pSwapChain, SyncInterval, Flags);
}


void d3d11_hook::hook_d3d11(d3d11_present_impl cb)
{
	p_present_impl = cb;

	// these are all things necessary to create a device and swap chain
	DummyWindow window = DummyWindow();

	auto feature_level = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window.m_hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = TRUE;

	IDXGISwapChain* p_swapchain;
	ID3D11Device* p_device;
	ID3D11DeviceContext* p_device_context;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &feature_level, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &p_swapchain, &p_device, NULL, &p_device_context);

	// Since a vtable is shared by all instances of the same class, we can use our 
	// dummy device/swapchain to get the address of the present function used by the d3d11 program's swapchain
	// the Present function is used to render content to the display, so it is called every frame by every program
	// that uses d3d11
	g_p_present = (d3d11_Present)get_VF((DWORD)p_swapchain, 8);

	// with the address of the Present function, we can apply a detour hook so that the Present function will
	// immediately jump to our present_callback function. From there we jump to the post_detour code cave which contains
	// the original code replaced by the jump/detour hook and then from there we jump back to the original Present function
	post_detour_cave = detour_hook((DWORD)g_p_present, (DWORD)&present_callback, 5);
	p_post_present_hook = (d3d11_Present)post_detour_cave;

	// the dummy device, swapchain ... isn't necessary anymore -> we only needed to get the address of the Present function
	p_device->Release();
	p_device_context->Release();
	p_swapchain->Release();

	while (!g_p_device) { Sleep(10); }
}

void d3d11_hook::unhook_d3d11()
{
	// restore the Present detour hook by writing the original code back to its original place in the Present function
	remove_detour_hook((DWORD)g_p_present, post_detour_cave, 5);
	if (post_detour_cave)
		delete[] post_detour_cave;
}
