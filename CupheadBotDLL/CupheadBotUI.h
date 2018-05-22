#pragma once
#include <Windows.h>
#include "d3d11_hook.h"
#include "CupheadBot.h"


class CupheadBotUI
{
public:
	CupheadBotUI(HMODULE dll_handle);

	// For the input handler and present_impl
	typedef LRESULT(CALLBACK *wndproc)(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
	wndproc orig_wndproc;

	static LRESULT CALLBACK input_handler(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
	static bool present_impl(ID3D11Device* device, ID3D11DeviceContext* device_context, IDXGISwapChain* swap_chain);

	void render_ui();

	void init_ui(ID3D11Device* p_device, ID3D11DeviceContext* p_device_context);
	void exit_ui();

	void hook_input_handler();
	void unhook_input_handler();

	void toggle_visibility() { ui_visible = !ui_visible; }

	bool is_visible() const { return ui_visible; }
	bool is_exit_scheduled() const { return exit_scheduled; }
	bool is_imgui_initialized() const { return imgui_initialized; }
private:
	void render_diagnostics();
	void show_error_tooltip(bool error);

	// UI flags
	bool ui_visible;
	bool ui_wallhack_enabled;
	bool ui_demo_visible;
	bool ui_invincibility;
	bool ui_infinite_jumping;
	bool ui_infinite_dashing;
	bool ui_infinite_damage;

	int ui_primary_weapon_idx;
	int ui_secondary_weapon_idx;
	int ui_money;

	// ImGUI flags
	bool imgui_initialized;

	// Hooking
	CupheadBot bot;

	bool exit_scheduled;
};
