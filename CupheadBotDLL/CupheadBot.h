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
	bool set_invincible(bool invincible) { return player_controller.set_invincible(invincible); }
	bool set_hp_to_max() { return player_controller.set_hp(player_controller.get_max_hp()); }
	bool set_primary_weapon(const PlayerControllerBot::Weapon& weapon) { return player_controller.set_primary_weapon(weapon); }
	bool set_secondary_weapon(const PlayerControllerBot::Weapon& weapon) { return player_controller.set_secondary_weapon(weapon); }

	// Cuphead manipulation hacks
	bool set_infinite_jumping(bool inf_jump);
	bool set_infinite_damage(bool inf_dmg);

	HMODULE get_dll_module() const { return dll_module; }
	HMODULE get_cuphead_module() const { return cuphead_module; }
	HWND get_cuphead_window_handle() const { return cuphead_window_handle; }

	const BasicHookInfo get_infinite_damage_info() const { return infinite_damage_info; }
	const BasicHookInfo get_infinite_jump_info() const { return infinite_jump_info; }

	static DWORD original_infinite_damage_func;
private:
	DWORD get_infinite_jumping_address();
	DWORD get_infinite_damage_address();
	DWORD get_money_address();

	PlayerControllerBot player_controller;

	// Hook data
	BasicHookInfo infinite_jump_info;
	BasicHookInfo infinite_damage_info;
	static DWORD money_function_adr;

	// Common handles
	HMODULE dll_module;
	HMODULE cuphead_module;
	HWND cuphead_window_handle;
};