// TODO update() function to automatically apply scheduled hacks


#pragma once
#include "memory_tools.h"
#include "PlayerControllerBot.h"

class CupheadBot
{
public:
	CupheadBot(HMODULE dll_handle);

	void exit();

	// Map hacks
	/** Allows walking through walls on the map/level selection screen. 
		Note: must be on the level selection screen. */
	void wallhack(bool enable);
	DWORD get_money();
	bool set_money(DWORD money);
	
	// Player controller hacks
	DWORD get_player_controller_address() { return player_controller.get_base_address(); }
	/* Note: must be in a level. */
	bool set_invincible_flag(bool invincible) { return player_controller.set_invincible(invincible); }
	bool set_hp_to_max();
	bool set_super_meter_to_max();
	bool set_super_no_cost(bool enable) { return player_controller.set_super_cost(enable ? 0 : PlayerControllerBot::DEFAULT_SUPER_COST); }
	bool set_primary_weapon(const PlayerControllerBot::Weapon& weapon) { return player_controller.set_primary_weapon(weapon); }
	bool set_secondary_weapon(const PlayerControllerBot::Weapon& weapon) { return player_controller.set_secondary_weapon(weapon); }

	// Cuphead manipulation hacks
	bool set_infinite_jumping(bool inf_jump);
	bool set_infinite_dashing(bool inf_dash);
	bool set_infinite_damage(bool inf_dmg);

	bool set_invincible(bool invincible);

	HMODULE get_dll_module() const { return dll_module; }
	HMODULE get_cuphead_module() const { return cuphead_module; }
	HWND get_cuphead_window_handle() const { return cuphead_window_handle; }

	const BasicHookInfo get_infinite_damage_info() const { return infinite_damage_info; }
	const BasicHookInfo get_infinite_jump_info() const { return infinite_jump_info; }
	const BasicHookInfo get_infinite_parry_info() const { return infinite_parry_info; }
	DWORD get_invincible_adr() const { return invincible_adr; }
	DWORD get_money_function_adr() const { return money_function_adr; }
	DWORD get_infinite_dash_adr() const { return infinite_dash_adr; }

	static DWORD original_infinite_damage_func;
private:
	/* Helper function that sets dst to find_signature() if necessary and returns the success of the operation. */
	bool set_signature_address(DWORD& dst, const BYTE signature[], size_t size);

	bool set_infinite_parry(bool inf_parry);

	bool update_super_meter_hud();
	bool update_hp_hud();

	DWORD get_money_address();

	PlayerControllerBot player_controller;

	// Hook data
	BasicHookInfo infinite_jump_info, infinite_parry_info;
	BasicHookInfo infinite_damage_info;
	DWORD invincible_adr;
	DWORD infinite_dash_adr;
	static DWORD money_function_adr;
	static DWORD hud_super_meter_update_adr, hud_hp_update_adr;

	// Common handles
	HMODULE dll_module;
	HMODULE cuphead_module;
	HWND cuphead_window_handle;
};