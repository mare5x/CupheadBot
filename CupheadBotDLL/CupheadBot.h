#pragma once
#include "memory_tools.h"
#include "PlayerControllerBot.h"

class CupheadBot
{
public:
	CupheadBot(HMODULE dll_handle);

	void exit();

	/** Allows walking through walls on the map/level selection screen. 
		Note: must be on the level selection screen. */
	void wallhack(bool enable);
	
	/* Note: must be in a level. */
	void set_invincible(bool invincible) { player_controller.set_invincible(invincible); }
	void set_hp_to_max() { player_controller.set_hp(player_controller.get_max_hp()); }

	HMODULE get_dll_module() const { return dll_module; }
	HMODULE get_cuphead_module() const { return cuphead_module; }
	HWND get_cuphead_window_handle() const { return cuphead_window_handle; }
private:
	PlayerControllerBot player_controller;

	HMODULE dll_module;
	HMODULE cuphead_module;
	HWND cuphead_window_handle;
};