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

	ui_money = bot.get_money();
}

void CupheadBotUI::render_ui()
{
	ImGui::Begin("CupheadBot", &ui_visible);

	if (ImGui::CollapsingHeader("Diagnostics"))
		render_diagnostics();

	static bool wallhack_failed = false;
	if (ImGui::Checkbox("Wallhack", &ui_wallhack_enabled)) {
		if (wallhack_failed = !bot.wallhack(ui_wallhack_enabled))
			ui_wallhack_enabled = !ui_wallhack_enabled;
	}
	show_error_tooltip(wallhack_failed);

	static bool set_money_failed = false;
	if (ImGui::InputInt("Money", &ui_money, 1, 5)) {
		set_money_failed = !bot.set_money(ui_money);
	}
	show_error_tooltip(set_money_failed);

	ImGui::SameLine(); ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Current money: %d", bot.get_money());
		ImGui::EndTooltip();
	}

	static bool invincible_failed = false;  // the static variable gets initialized only the first time around
	if (ImGui::Checkbox("Invincible", &ui_invincibility)) {
		if (invincible_failed = !bot.set_invincible(ui_invincibility))
			ui_invincibility = !ui_invincibility;
	}
	show_error_tooltip(invincible_failed);

	static bool max_hp_failed = false;
	if (ImGui::Button("Max HP")) {
		max_hp_failed = !bot.set_hp_to_max();
	}
	show_error_tooltip(max_hp_failed);

	static bool max_super_failed = false;
	if (ImGui::Button("Max Super Meter")) {
		max_super_failed = !bot.set_super_meter_to_max();
	}
	show_error_tooltip(max_super_failed);

	static bool no_cost_super_failed = false;
	if (ImGui::Checkbox("No cost Super", &ui_no_cost_super)) {
		if (no_cost_super_failed = !bot.set_super_no_cost(ui_no_cost_super))
			ui_no_cost_super = !ui_no_cost_super;
	}
	show_error_tooltip(no_cost_super_failed);

	render_loadout();

	static bool inf_jump_failed = false;
	static bool inf_dashing_failed = false;
	if (ImGui::Checkbox("Infinite jumping", &ui_infinite_jumping)) {
		if (inf_jump_failed = !bot.set_infinite_jumping(ui_infinite_jumping))
			ui_infinite_jumping = !ui_infinite_jumping;
		else {
			ui_infinite_dashing = ui_infinite_jumping;
			if (inf_dashing_failed = !bot.set_infinite_dashing(ui_infinite_dashing))
				ui_infinite_dashing = !ui_infinite_dashing;
		}
	}
	show_error_tooltip(inf_jump_failed);

	if (ImGui::Checkbox("Infinite dashing", &ui_infinite_dashing)) {
		if (inf_dashing_failed = !bot.set_infinite_dashing(ui_infinite_dashing))
			ui_infinite_dashing = !ui_infinite_dashing;
	}
	show_error_tooltip(inf_dashing_failed);

	static bool inf_dmg_failed = false;
	if (ImGui::Checkbox("One punch man", &ui_infinite_damage)) {
		if (inf_dmg_failed = !bot.set_infinite_damage(ui_infinite_damage))
			ui_infinite_damage = !ui_infinite_damage;
	}
	show_error_tooltip(inf_dmg_failed);

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
	bot.exit();
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

	if (ui_ptr->is_visible()) {
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard) // || io.WantCaptureMouse)
			return true;
	}

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
}

void CupheadBotUI::render_diagnostics()
{
	ImGui::Text("Base CupheadBot.dll: %x", bot.get_dll_module());
	ImGui::Text("Base Cuphead.exe: %x", bot.get_cuphead_module());
	ImGui::Text("Original WNDPROC: %x, hooked input_handler: %x", orig_wndproc, &CupheadBotUI::input_handler);

	ImGui::Text("d3d11_hook::g_p_present: %x", d3d11_hook::g_p_present);
	ImGui::Text("present_impl: %x", &CupheadBotUI::present_impl);
	ImGui::Text("d3d11_hook::g_p_swapchain: %x", d3d11_hook::g_p_swapchain);
	ImGui::Text("d3d11_hook::g_p_device: %x", d3d11_hook::g_p_device);
	ImGui::Text("d3d11_hook::g_p_device_context: %x", d3d11_hook::g_p_device_context);
	
	ImGui::Text("PlayerController: %x", bot.get_player_controller().get_player_controller_address());
	ImGui::Text("PlayerStats: %x", bot.get_player_controller().get_stats_address());

	ImGui::Text("Infinite jump hook_at: %x", bot.get_infinite_jump_info().hook_at);
	ImGui::Text("Infinite parry hook_at: %x", bot.get_infinite_parry_info().hook_at);
	ImGui::Text("Infinite dash hook_at: %x", bot.get_infinite_dash_adr());
	ImGui::Text("Infinite damage hook_at: %x", bot.get_infinite_damage_info().hook_at);
	
	ImGui::Text("Invincible address hook_at: %x", bot.get_invincible_adr());

	ImGui::Text("PlayerData: %x", bot.get_player_data().get_player_data_address());
	ImGui::Text("Loadout address: %x", bot.get_player_data().get_loadout_address());
}

void CupheadBotUI::render_loadout()
{
	if (!bot.get_wallhack_adr()) {
		ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "LOAD THE GAME TO CHANGE LOADOUT");
		return;
	}

	static bool primary_weapon_failed = false;
	const auto& loadout = bot.get_player_data().get_loadout();
	const auto& weapon_table = loadout.WEAPON_TABLE;
	const LoadoutWeapon& selected_primary_weapon = loadout.get_primary_weapon();
	// Begin Combo list, add Selectables (items), highlight the already selected item and update the new selected item
	if (ImGui::BeginCombo("Primary weapon", selected_primary_weapon.name)) {
		for (size_t i = 0; i < weapon_table.size(); ++i) {
			// tell the Selectable if it was previously selected
			const auto& weapon = weapon_table[i];
			bool is_selected = (weapon == selected_primary_weapon);
			if (ImGui::Selectable(weapon.name, is_selected))
				primary_weapon_failed = !bot.set_primary_weapon(weapon);
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	show_error_tooltip(primary_weapon_failed);

	static bool secondary_weapon_failed = false;
	const LoadoutWeapon& selected_secondary_weapon = loadout.get_secondary_weapon();
	if (ImGui::BeginCombo("Secondary weapon", selected_secondary_weapon.name)) {
		for (size_t i = 0; i < weapon_table.size(); ++i) {
			const auto& weapon = weapon_table[i];
			bool is_selected = (selected_secondary_weapon == weapon);
			if (ImGui::Selectable(weapon.name, is_selected))
				secondary_weapon_failed = !bot.set_secondary_weapon(weapon);
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	show_error_tooltip(secondary_weapon_failed);

	static bool super_failed = false;
	const auto& super_table = PlayerDataBot::Loadout::SUPER_TABLE;
	const LoadoutSuper& selected_super = loadout.get_super();
	if (ImGui::BeginCombo("Super", selected_super.name)) {
		for (size_t i = 0; i < super_table.size(); ++i) {
			const auto& super = super_table[i];
			bool is_selected = (selected_super == super);
			if (ImGui::Selectable(super.name, is_selected))
				super_failed = !bot.set_super(super);
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	show_error_tooltip(super_failed);

	static bool charm_failed = false;
	const auto& charm_table = PlayerDataBot::Loadout::CHARM_TABLE;
	const LoadoutCharm& selected_charm = loadout.get_charm();
	if (ImGui::BeginCombo("Charm", selected_charm.name)) {
		for (size_t i = 0; i < charm_table.size(); ++i) {
			const auto& charm = charm_table[i];
			bool is_selected = (selected_charm == charm);
			if (ImGui::Selectable(charm.name, is_selected))
				charm_failed = !bot.set_charm(charm);
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	show_error_tooltip(charm_failed);
}

void CupheadBotUI::show_error_tooltip(bool error)
{
	if (!error) return;

	//static const ImVec4 success_color = { 0.0f, 1.0f, 0.0f, 1.0f };
	static const ImVec4 failed_color = { 1.0f, 0.0f, 0.0f, 1.0f };

	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextColored(failed_color, "ERROR!");
		ImGui::EndTooltip();
	}
}

/* Called from Cuphead.exe's thread. */
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
