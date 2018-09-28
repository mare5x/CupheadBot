// TODO update() function to automatically apply scheduled hacks


#pragma once
#include "memory_tools.h"
#include "PlayerControllerBot.h"
#include "PlayerDataBot.h"
#include "MonoWrapper.h"

class CupheadBot
{
public:
	CupheadBot(HMODULE dll_handle);

	void exit();
	
	// mono
	bool toggle_debug_console();
	bool log_mono_classes(bool full_name = false);
	bool log_mono_class_methods(const char* class_name);

	// Map hacks
	/** Allows walking through walls on the map/level selection screen. 
		Note: must be on the map. */
	bool wallhack(bool enable);
	DWORD get_wallhack_adr();
	
	// Player controller hacks
	bool set_invincible_flag(bool invincible) { return player_controller.set_invincible(invincible); } 	/* Note: must be in a level. */
	bool set_hp_to_max();
	bool set_super_meter_to_max();
	bool set_super_no_cost(bool enable) { return player_controller.set_super_cost(enable ? 0 : PlayerControllerBot::DEFAULT_SUPER_COST); }

	// PlayerData hacks (static)
	bool set_primary_weapon(const LoadoutWeapon & weapon);
	bool set_secondary_weapon(const LoadoutWeapon & weapon);
	bool set_weapon(const LoadoutWeapon & weapon);  /* Switch to weapon without modifying the loadout. */
	bool set_charm(const LoadoutCharm& charm);
	bool set_super(const LoadoutSuper& super);
	DWORD get_money() { return player_data.get_money(); }
	bool set_money(DWORD money) { return player_data.set_money(money); }

	// Cuphead manipulation hacks
	bool set_infinite_jumping(bool inf_jump);
	bool set_infinite_dashing(bool inf_dash);
	bool set_infinite_damage(bool inf_dmg);

	bool set_invincible(bool invincible);

	HMODULE get_dll_module() const { return dll_module; }
	HMODULE get_cuphead_module() const { return cuphead_module; }
	HWND get_cuphead_window_handle() const { return cuphead_window_handle; }

	PlayerControllerBot& get_player_controller() { return player_controller; }
	const BasicHookInfo get_infinite_damage_info() const { return infinite_damage_info; }
	const BasicHookInfo get_infinite_jump_info() const { return infinite_jump_info; }
	const BasicHookInfo get_infinite_parry_info() const { return infinite_parry_info; }
	DWORD get_invincible_adr() const { return invincible_adr; }
	DWORD get_infinite_dash_adr() const { return infinite_dash_adr; }
	PlayerDataBot& get_player_data() { return player_data; }

	MonoWrapper& get_mono_wrapper() { return mono; }

	static DWORD original_infinite_damage_func;
private:
	/* Helper function that sets dst to find_signature() if necessary and returns the success of the operation. */
	bool set_signature_address(DWORD& dst, const BYTE signature[], size_t size);

	bool set_infinite_parry(bool inf_parry);

	bool update_super_meter_hud();
	bool update_hp_hud();

	PlayerControllerBot player_controller;
	PlayerDataBot player_data;

	// Hook data
	BasicHookInfo infinite_jump_info, infinite_parry_info;
	BasicHookInfo infinite_damage_info;
	DWORD invincible_adr;
	DWORD infinite_dash_adr;
	static DWORD switch_weapon_function_adr;
	static DWORD hud_super_meter_update_adr, hud_hp_update_adr;

	// Mono
	MonoWrapper mono;
	void* mono_image;  // Assembly-CSharp image

	// Common handles
	HMODULE dll_module;
	HMODULE cuphead_module;
	HWND cuphead_window_handle;
};