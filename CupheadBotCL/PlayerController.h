#pragma once
#include "memory_tools.h"
#include "InjectionHelper.h"


class PlayerController {
public:
	enum WEAPON {
		PEASHOOTER = 1456773641,
		SPREAD = 1456773649,
		CHASER = 1460621839,
		LOBBER = 1467024095,
		CHARGE = 1466416941,
		ROUNDABOUT = 1466518900
	};

	PlayerController(HANDLE process, DWORD _base_p = 0);
	// Calls quit(), but make sure that the handle is still valid (i.e. that the destructor is called before CloseHandle()).
	~PlayerController() { quit(); }

	bool initialized() const { return base_p; }

	// Restores changes made to Cuphead.exe.
	void quit();

	void set_hp(DWORD new_hp) const { write_memory<DWORD>(proc, hp_p, new_hp); }
	DWORD get_hp() const { return read_memory<DWORD>(proc, hp_p); }

	void toggle_invincibility() { set_hard_invincibility(!is_invincible); }
	void set_hard_invincibility(bool new_val) { is_invincible = new_val;  write_memory<BYTE>(proc, super_invincibility_p, new_val); }
	bool read_hard_invincibility() const { return read_memory<BYTE>(proc, super_invincibility_p); }

	void set_primary_weapon(WEAPON weapon) const { write_memory<DWORD>(proc, primary_weapon_p, static_cast<DWORD>(weapon)); }
	void set_secondary_weapon(WEAPON weapon) const { write_memory<DWORD>(proc, secondary_weapon_p, static_cast<DWORD>(weapon)); }

	void toggle_infinite_damage();
	void toggle_inf_jumping();

	bool infinite_jumping_enabled() const { return infinite_jumping; }
private:
	void init_player_controller();

	DWORD get_player_controller_address();
	DWORD get_jump_nop_address();
	DWORD get_inf_dmg_address();

	HANDLE proc;

	DWORD base_p, hp_p, hp_max_p;
	DWORD primary_weapon_p, secondary_weapon_p;
	DWORD super_invincibility_p;

	InjectionHelper inf_dmg_info, inf_jump_info;

	bool infinite_jumping, infinite_damage, is_invincible;
};