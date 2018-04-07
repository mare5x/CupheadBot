#pragma once
#include "memory_tools.h"


class PlayerController {
public:
	PlayerController(HANDLE process, DWORD _base_p = 0);

	void set_hp(DWORD new_hp) { write_memory<DWORD>(proc, hp_p, new_hp); }
	DWORD get_hp() { return read_memory<DWORD>(proc, hp_p); }
private:
	DWORD get_player_controller_address();

	HANDLE proc;
	DWORD base_p;

	DWORD hp_p, hp_max_p;
};