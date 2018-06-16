#include "CupheadBot.h"
#include "LoggerUI.h"

DWORD CupheadBot::original_infinite_damage_func = 0;
DWORD CupheadBot::switch_weapon_function_adr = 0;
DWORD CupheadBot::hud_super_meter_update_adr = 0;
DWORD CupheadBot::hud_hp_update_adr = 0;


CupheadBot::CupheadBot(HMODULE dll_handle)
	:dll_module(dll_handle)
{
	cuphead_window_handle = FindWindow(NULL, L"Cuphead");
	cuphead_module = GetModuleHandle(NULL);

	player_controller.init();

	infinite_jump_info.original_bytes = { 0x0F, 0x85, 0xBC, 0x01, 0x00, 0x00 };
	infinite_parry_info.original_bytes = { 0x8B, 0x47, 0x58, 0xC7, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x47, 0x4C, 0xC7, 0x40, 0x08, 0x01, 0x00, 0x00, 0x00 };
	infinite_damage_info.original_bytes = { 0x8B, 0x46, 0x38, 0x83, 0xEC, 0x08 };
}

void CupheadBot::exit()
{
	if (infinite_jump_info.hook_at)
		set_infinite_jumping(false);
	if (infinite_dash_adr)
		set_infinite_dashing(false);
	if (infinite_damage_info.hook_at)
		set_infinite_damage(false);
	if (invincible_adr)
		set_invincible(false);
	player_controller.exit();
	mono.detach();
}

/** If DebugConsole::Show is called, the game will crash when exiting to desktop!
	NOTE: Must not be on the title screen to work. 
*/
bool CupheadBot::toggle_debug_console()
{
	void* image = mono.find_image("Assembly-CSharp");
	void* debug_console_class = mono.get_class(image, "DebugConsole");

	// alternate way to find methods:
	//void* method = mono.get_method_image("DebugConsole:Hide", image);

	void* console_is_visible_method = mono.get_method_class(debug_console_class, "get_IsVisible");
	bool is_console_visible = mono.runtime_invoke<bool>(console_is_visible_method);  // must be on map to work

	if (is_console_visible) {
		void* hide_console_method = mono.get_method_class(debug_console_class, "Hide");
		void* hide_console_adr = mono.jit_method(hide_console_method);
		ui_logger::log("DebugConsole::Hide() (0x%x) -> 0x%x\n", hide_console_method, hide_console_adr);
		if (hide_console_adr) {
			__asm { call[hide_console_adr] }
		}
		else {
			ui_logger::error("ERROR: hide console failed!\n");
			return false;
		}
	}
	else {
		void* show_console_method = mono.get_method_class(debug_console_class, "Show");
		void* show_console_adr = mono.jit_method(show_console_method);
		ui_logger::log("DebugConsole::Show() (0x%x) -> 0x%x\n", show_console_method, show_console_adr);

		if (show_console_adr) {
			__asm { call[show_console_adr] }
		}
		else {
			ui_logger::error("ERROR: show console failed!\n");
			return false;
		}
	}
	return true;
}

bool CupheadBot::log_mono_class_methods(const char * class_name)
{
	void* image = mono.find_image("Assembly-CSharp");
	void* klass = mono.get_class(image, class_name);
	if (!klass) {
		ui_logger::error("%s not found!\n", class_name);
		return false;
	}
	ui_logger::log("%s (0x%x) methods:\n", class_name, klass);
	for (auto method : mono.get_methods(klass)) {
		ui_logger::log("0x%x -> %s\n", method, mono.get_method_name(method));
	}
	return true;
}

bool CupheadBot::wallhack(bool enable)
{
	DWORD adr = get_wallhack_adr();
	if (adr)
		write_memory<DWORD>(adr, enable ? 1 : 2);
	return adr;
}

/* The chain should always be valid, except on the title screen. */
DWORD CupheadBot::get_wallhack_adr()
{
	DWORD ptr_chain = read_memory<DWORD>((DWORD)cuphead_module + 0x104FA20);
	if (!ptr_chain) return 0;
	ptr_chain = read_memory<DWORD>(ptr_chain + 4);
	if (!ptr_chain) return 0;
	ptr_chain = read_memory<DWORD>(ptr_chain + 0x44);
	if (!ptr_chain) return 0;
	return ptr_chain;
}

bool CupheadBot::update_super_meter_hud()
{
	static const BYTE signature[] = {
		0x55, 0x8B, 0xEC, 0x56, 0x83, 0xEC, 0x14, 0x8B, 0x75, 0x08, 0xD9, 0x46, 0x68, 0xD9, 0x5D, 0xF8, 0xD9, 0x45, 0xF8, 0xD9, 0xEE, 0xD9, 0x46, 0x64, 0xD9, 0x5D, 0xF8, 0xD9, 0x45, 0xF8
	};
	if (!set_signature_address(hud_super_meter_update_adr, signature, sizeof(signature)))
		return false;

	DWORD pc = player_controller.get_stats_address();
	if (!pc)
		return false;

	__asm {
		sub esp, 0x8
		push 1
		push [pc]
		call [hud_super_meter_update_adr]
		add esp, 0x10
	}
	return true;
}

bool CupheadBot::update_hp_hud()
{
	static const BYTE signature[] = {
		0x55, 0x8B, 0xEC, 0x57, 0x83, 0xEC, 0x14, 0x8B, 0x7D, 0x08, 0x8B, 0x47, 0x60, 0x8B, 0x4F, 0x5C, 0x83, 0xEC, 0x04, 0x51
	};
	if (!set_signature_address(hud_hp_update_adr, signature, sizeof(signature)))
		return false;

	DWORD pc = player_controller.get_stats_address();
	if (!pc)
		return false;

	__asm {
		sub esp, 0xC
		push [pc]
		call [hud_hp_update_adr]
		add esp, 0x10
	}
	return true;
}

bool CupheadBot::set_hp_to_max()
{
	if (player_controller.set_hp(player_controller.get_max_hp()))
		return update_hp_hud();
	return false;
}

bool CupheadBot::set_super_meter_to_max()
{
	if (player_controller.set_super_meter(1000.0f)) // the game automatically bound checks anyways ...
		return update_super_meter_hud();
	return false;
}

bool CupheadBot::set_primary_weapon(const LoadoutWeapon & weapon)
{
	PlayerDataBot::Loadout loadout = player_data.get_loadout();
	if (loadout.is_valid()) {
		loadout.set_primary_weapon(weapon);
		return set_weapon(weapon);
	}
	return false;
}

bool CupheadBot::set_secondary_weapon(const LoadoutWeapon & weapon)
{
	PlayerDataBot::Loadout loadout = player_data.get_loadout();
	if (loadout.is_valid()) {
		loadout.set_secondary_weapon(weapon);
		return set_weapon(weapon);
	}
	return false;
}

bool CupheadBot::set_charm(const LoadoutCharm & charm)
{
	PlayerDataBot::Loadout loadout = player_data.get_loadout();
	if (loadout.is_valid()) {
		loadout.set_charm(charm);
		return true;
	}
	return false;
}

bool CupheadBot::set_super(const LoadoutSuper & super)
{
	PlayerDataBot::Loadout loadout = player_data.get_loadout();
	if (loadout.is_valid()) {
		loadout.set_super(super);
		return true;
	}
	return false;
}

bool CupheadBot::set_weapon(const LoadoutWeapon & weapon)
{
	static const BYTE signature[] = {
		0x83, 0xC4, 0x10, 0xE9, 0xAA, 0x00, 0x00, 0x00, 0x8B, 0x46, 0x3C, 0x8B, 0x8E, 0x90, 0x00, 0x00, 0x00, 0x83, 0xEC, 0x08, 0x51, 0x50, 0x39, 0x00
	};
	if (!switch_weapon_function_adr) {
		if (switch_weapon_function_adr = find_signature(signature, sizeof(signature)))
			switch_weapon_function_adr -= 0x50;  // find a simple signature inside the function and then subtract to get the base address of the function
		else
			return false;
	}
	DWORD weapon_manager_adr = player_controller.get_weapon_manager_address();
	if (!weapon_manager_adr)
		return false;
	DWORD weapon_id = weapon.id;
	__asm {
		sub esp, 8
		push [weapon_id]
		push [weapon_manager_adr]
		call [switch_weapon_function_adr]
		add esp, 0x10
	}

	return true;
}

bool CupheadBot::set_infinite_jumping(bool inf_jump)
{
	static const BYTE signature[] = {
		0x0F, 0x85, 0xBC, 0x01, 0x00, 0x00,
		0x8B, 0x47, 0x38, 0x83, 0xEC, 0x0C, 0x50, 0x39, 0x00
	};
	// get address to NOP
	if (!set_signature_address(infinite_jump_info.hook_at, signature, sizeof(signature)))
		return false;

	if (inf_jump) {
		// nop address
		nop_fill(infinite_jump_info.hook_at, infinite_jump_info.original_bytes.size());
	}
	else {
		write_code_buffer(infinite_jump_info.hook_at, infinite_jump_info.original_bytes.data(), infinite_jump_info.original_bytes.size());
	}
	return set_infinite_parry(inf_jump);
}

bool CupheadBot::set_infinite_dashing(bool inf_dash)
{
	static const BYTE signature[] = {
		0xC7, 0x40, 0x08, 0x04, 0x00, 0x00, 0x00, 0x8B, 0x47, 0x48, 0xD9, 0xEE, 0xD9, 0x58, 0x10, 0x8B, 0x47, 0x40, 0xD9, 0xEE, 0xD9, 0x58, 0x1C, 0x8B, 0x47, 0x7C
	};
	if (!set_signature_address(infinite_dash_adr, signature, sizeof(signature)))
		return false;
	
	// values: 0 (not dashing), 1 (dash started), 2 (currently dashing), 4 (dash finished)
	// normally the process goes from: 1 -> 2 -> 4 -> 0 (0 is set once cuphead hits the ground)
	// now it goes:					   1 -> 2 -> 0
	
	// Change this instruction in the function that gets called when dashing is complete
	// C7 40 08 04000000 - mov[eax + 08], 00000004
	//          ^^
	// change only the 3-rd byte
	if (inf_dash) {
		write_memory<BYTE>(infinite_dash_adr + 3, 0);
	}
	else {
		write_memory<BYTE>(infinite_dash_adr + 3, 4);
	}
	return true;
}

/* Trampoline function that sets the projectile damage to infinity (9999) and jumps back to the original code. */
void __declspec(naked) infinite_damage_setter()
{
	__asm {
		mov    DWORD PTR[edi+0x8], 0x461c3c00
		mov    eax, DWORD PTR[esi + 0x38]							// code replaced by the JMP hook
		sub    esp, 0x8
		JMP [CupheadBot::original_infinite_damage_func]				// jump back to original code
	}
}

bool CupheadBot::set_infinite_damage(bool inf_dmg)
{
	static const BYTE signature[] = {
		0x8B, 0x46, 0x38, 0x83, 0xEC, 0x08, 0x57, 0x50, 0x90, 0x90, 0x90, 0xFF, 0x50, 0x0C, 0x83, 0xC4, 0x10, 0x8B, 0x46, 0x3C
	};
	if (!set_signature_address(infinite_damage_info.hook_at, signature, sizeof(signature)))
		return false;
	
	if (inf_dmg)
		CupheadBot::original_infinite_damage_func = jump_hook(infinite_damage_info.hook_at, 
															  (DWORD)&infinite_damage_setter, 
															  infinite_damage_info.original_bytes.size());
	else
		jump_unhook(infinite_damage_info.hook_at, infinite_damage_info.original_bytes.data(), infinite_damage_info.original_bytes.size());
	return true;
}

/** Ignores damage.
	Note: the player must receive damage for the code to get JITted. */
bool CupheadBot::set_invincible(bool invincible)
{
	static const BYTE signature[] = {
		0x0F, 0x85, 0xE5, 0x00, 0x00, 0x00, 0x39, 0x3F, 0xD9, 0x47, 0x08, 0xD9, 0x5D, 0xF0, 0xD9, 0x45, 0xF0, 0xD9, 0xEE, 0xDF, 0xF1
	};
	if (!set_signature_address(invincible_adr, signature, sizeof(signature)))
		return false;

	// The difference in invincibility is a single byte -> JNE (0x85) to JE (0x84)
	// in the damage receiver function.
	if (invincible) {
		write_memory<BYTE>(invincible_adr + 1, 0x84);
	}
	else {
		write_memory<BYTE>(invincible_adr + 1, 0x85);
	}
	return true;
}

bool CupheadBot::set_infinite_parry(bool inf_parry)
{
	static const BYTE signature[] = {
		0x8B, 0x47, 0x58, 0xC7, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x47, 0x4C, 0xC7, 0x40, 0x08, 0x01, 0x00, 0x00, 0x00,
		0x8B, 0x47, 0x48, 0x83, 0xEC, 0x0C, 0x50, 0x39, 0x00, 0xE8
	};
	if (!set_signature_address(infinite_parry_info.hook_at, signature, sizeof(signature)))
		return false;

	if (inf_parry) {
		nop_fill(infinite_parry_info.hook_at, infinite_parry_info.original_bytes.size());
	}
	else {
		write_code_buffer(infinite_parry_info.hook_at, infinite_parry_info.original_bytes.data(), infinite_parry_info.original_bytes.size());
	}
	return true;
}

bool CupheadBot::set_signature_address(DWORD & dst, const BYTE signature[], size_t size)
{
	if (!dst) {
		dst = find_signature(signature, size);
		if (!dst)
			return false;
	}
	return true;
}