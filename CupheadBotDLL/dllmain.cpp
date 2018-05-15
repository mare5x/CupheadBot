#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"
#include "memory_tools.h"
#include "d3d11_hook.h"


HMODULE g_cuphead_module;
HMODULE g_dll_module;
HWND g_cuphead_window_handle;

bool g_imgui_initialized = false;
bool g_exit_scheduled = false;

typedef LRESULT(CALLBACK *wndproc)(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
wndproc g_orig_wndproc_handler = nullptr;

// UI flags
bool ui_wallhack_enabled = false;
bool ui_visible = true;
bool ui_demo_visible = false;


// defined in imgui_impl_dx11.cpp
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms644927(v=vs.85).aspx
LRESULT CALLBACK window_proc_impl(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	// F1 KEY TOGGLES UI VISIBILITY
	if (uMsg == WM_KEYDOWN && wParam == VK_F1) {
		ui_visible = !ui_visible;
	}

	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	return g_orig_wndproc_handler(hwnd, uMsg, wParam, lParam);
}


void hook_input_handler()
{
	// hook Cuphead's input handler and instead use my own version which is used by ImGui
	g_orig_wndproc_handler = (wndproc)GetWindowLongPtr(g_cuphead_window_handle, GWLP_WNDPROC);
	SetWindowLongPtr(g_cuphead_window_handle, GWLP_WNDPROC, (LONG_PTR)&window_proc_impl);
}


void unhook_input_handler()
{
	SetWindowLongPtr(g_cuphead_window_handle, GWLP_WNDPROC, (LONG_PTR)g_orig_wndproc_handler);
	g_orig_wndproc_handler = nullptr;
}


void init_overlay()
{
	ImGui::CreateContext();
	//ImGuiIO &io = ImGui::GetIO();
	g_imgui_initialized = ImGui_ImplDX11_Init(g_cuphead_window_handle, g_p_device, g_p_device_context);

	hook_input_handler();
}


void exit_overlay()
{
	g_imgui_initialized = false;
	unhook_input_handler();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
}


void render_ui()
{
	ImGui::Begin("CupheadBot", &ui_visible);

	ImGui::Text("BASE DLL: %x", g_dll_module);
	ImGui::Text("BASE CUPHEAD.EXE: %x", g_cuphead_module);
	ImGui::Text("WNDPROC: %x, HOOKED: %x", g_orig_wndproc_handler, &window_proc_impl);

	if (ImGui::Checkbox("Wallhack", &ui_wallhack_enabled)) {
		DWORD ptr0 = read_memory<DWORD>((DWORD)g_cuphead_module + 0x104FA20);
		DWORD ptr1 = read_memory<DWORD>(ptr0 + 4);
		DWORD cuphead_world_player_ptr = read_memory<DWORD>(ptr1 + 0x44);
		write_memory<DWORD>(cuphead_world_player_ptr, ui_wallhack_enabled ? 1 : 2);
	}

	if (ImGui::Button("SHOW DEMO WINDOW"))
		ui_demo_visible = !ui_demo_visible;

	if (ui_demo_visible)
		ImGui::ShowDemoWindow(&ui_demo_visible);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
	if (ImGui::Button("EXIT")) {                           // Buttons return true when clicked (NB: most widgets return true when edited/activated)
		g_exit_scheduled = true;
	}

	ImGui::End();
}


bool present_impl(ID3D11Device* device, ID3D11DeviceContext* device_context, IDXGISwapChain* swap_chain)
{
	if (g_exit_scheduled) {
		// the reason unhooking is done here is because otherwise we wouldn't know what Cuphead's thread is executing
		// this way at least we can control exactly what is happening
		exit_overlay();
		return true;
	}
	if (!g_imgui_initialized || !ui_visible) return false;

	ImGui_ImplDX11_NewFrame();

	render_ui();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return false;
}


DWORD WINAPI run_bot(LPVOID param = NULL)
{
	g_cuphead_window_handle = FindWindow(NULL, L"Cuphead");
	g_cuphead_module = GetModuleHandle(NULL);
	
	hook_d3d11();

	init_overlay();

	while (!g_exit_scheduled)
		Sleep(500);
	Sleep(200);  // just wait, since we don't know what the thread is currently executing ...
	FreeLibraryAndExitThread(g_dll_module, NULL);

	return 1;
}


//DWORD WINAPI on_exit(LPVOID param = NULL)
//{
//	g_exit_scheduled = true;
//	Sleep(200);
//
//	return 1;
//}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		g_dll_module = hModule;
		HANDLE thread = CreateThread(NULL, NULL, &run_bot, NULL, NULL, NULL);
		CloseHandle(thread);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		//HANDLE thread = CreateThread(NULL, NULL, &on_exit, NULL, NULL, NULL);
		//CloseHandle(thread);
		//on_exit();
	}
    return TRUE;
}