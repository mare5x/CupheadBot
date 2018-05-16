#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"
#include "CupheadBotUI.h"


// defined in imgui_impl_dx11.cpp
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

CupheadBotUI* ui_ptr;  // yikes 0w0


CupheadBotUI::CupheadBotUI(HMODULE dll_handle)
	:bot(dll_handle)
{
	ui_ptr = this;

	ui_visible = true;

	d3d11_hook::hook_d3d11(&CupheadBotUI::present_impl);
	init_ui(d3d11_hook::g_p_device, d3d11_hook::g_p_device_context);
}

void CupheadBotUI::render_ui()
{
	ImGui::Begin("CupheadBot", &ui_visible);

	ImGui::Text("BASE DLL: %x", bot.get_dll_module());
	ImGui::Text("BASE CUPHEAD.EXE: %x", bot.get_cuphead_module());
	ImGui::Text("WNDPROC: %x, HOOKED: %x", orig_wndproc, &CupheadBotUI::input_handler);

	if (ImGui::Checkbox("Wallhack", &ui_wallhack_enabled)) {
		bot.wallhack(ui_wallhack_enabled);
	}

	if (ImGui::Button("SHOW DEMO WINDOW"))
		ui_demo_visible = !ui_demo_visible;

	if (ui_demo_visible)
		ImGui::ShowDemoWindow(&ui_demo_visible);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (ImGui::Button("EXIT")) {
		exit_scheduled = true;
	}

	ImGui::End();
}

void CupheadBotUI::init_ui(ID3D11Device* p_device, ID3D11DeviceContext* p_device_context)
{
	ImGui::CreateContext();
	imgui_initialized = ImGui_ImplDX11_Init(bot.get_cuphead_window_handle(), p_device, p_device_context);

	hook_input_handler();
}

void CupheadBotUI::exit_ui()
{
	imgui_initialized = false;
	unhook_input_handler();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms644927(v=vs.85).aspx
LRESULT CALLBACK CupheadBotUI::input_handler(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	// F1 KEY TOGGLES UI VISIBILITY
	if (uMsg == WM_KEYDOWN && wParam == VK_F1) {
		ui_ptr->toggle_visibility();
	}

	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	return ui_ptr->orig_wndproc(hwnd, uMsg, wParam, lParam);
}

void CupheadBotUI::hook_input_handler()
{
	// hook Cuphead's input handler and instead use my own version which is used by ImGui
	orig_wndproc = (wndproc)GetWindowLongPtr(bot.get_cuphead_window_handle(), GWLP_WNDPROC);
	SetWindowLongPtr(bot.get_cuphead_window_handle(), GWLP_WNDPROC, (LONG_PTR)&CupheadBotUI::input_handler);
}

void CupheadBotUI::unhook_input_handler()
{
	SetWindowLongPtr(bot.get_cuphead_window_handle(), GWLP_WNDPROC, (LONG_PTR)orig_wndproc);
	orig_wndproc = nullptr;
}


bool CupheadBotUI::present_impl(ID3D11Device* device, ID3D11DeviceContext* device_context, IDXGISwapChain* swap_chain)
{
	CupheadBotUI& ui = *ui_ptr;

	if (ui.is_exit_scheduled()) {
		// the reason unhooking is done here is because otherwise we wouldn't know what Cuphead's thread is executing
		// this way at least we can control exactly what is happening
		ui.exit_ui();
		return true;
	}
	if (!ui.is_imgui_initialized() || !ui.is_visible()) return false;

	ImGui_ImplDX11_NewFrame();

	ui.render_ui();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return false;
}