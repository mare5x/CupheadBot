#pragma once
#include "memory_tools.h"


class PlayerController {
public:
	PlayerController(HANDLE process, DWORD _base_p = 0);

	bool initialized() const { return base_p; }

	void set_hp(DWORD new_hp) const { write_memory<DWORD>(proc, hp_p, new_hp); }
	DWORD get_hp() const { return read_memory<DWORD>(proc, hp_p); }

	void set_hard_invincibility(bool new_val) const { write_memory<BYTE>(proc, hard_invincibility_p, new_val); }
	bool get_hard_invincibility() const { return read_memory<BYTE>(proc, hard_invincibility_p); }

	void toggle_inf_jumping();

	bool infinite_jumping_enabled() const { return infinite_jumping; }
private:
	DWORD get_player_controller_address();
	DWORD get_jump_nop_address();

	HANDLE proc;
	DWORD base_p;
	DWORD jump_nop_address;

	DWORD hp_p, hp_max_p;
	DWORD hard_invincibility_p;

	bool infinite_jumping;
};