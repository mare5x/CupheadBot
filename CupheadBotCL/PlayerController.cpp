#include <iostream>
#include "PlayerController.h"


PlayerController::PlayerController(HANDLE process, DWORD _base_p)
	: proc(process), base_p(_base_p)
{
	init_player_controller();

	inf_dmg_info.original_code = { 0x8B, 0x46, 0x38, 0x83, 0xEC, 0x08 };
	inf_dmg_info.shellcode = {
		0xC7, 0x47, 0x08, 0x00, 0x3C, 0x1C, 0x46,	// mov    DWORD PTR [edi+0x8],0x461c3c00  9999
		0x8B, 0x46, 0x38, 0x83, 0xEC, 0x08,			// code replaced by the JMP hook
		0xE9, 0x00, 0x00, 0x00, 0x00				// jump back to original code
	};

	inf_jump_info.original_code = { 0x0F, 0x85, 0xBC, 0x01, 0x00, 0x00 };
}

void PlayerController::quit()
{
	inf_dmg_info.free_code_cave(proc);
	inf_dmg_info.restore_original_code(proc);

	inf_jump_info.restore_original_code(proc);
}


/* NOTE: the code gets JITted only after the first hit in game. */
void PlayerController::toggle_infinite_damage()
{
	if (!inf_dmg_info.hook_at_adr) {
		inf_dmg_info.hook_at_adr = get_inf_dmg_address();
		if (!inf_dmg_info.hook_at_adr) {
			std::cout << "INVALID INFINITE DAMAGE JUMP ADDRESS!\n";
			return;
		}
	}

	infinite_damage = !infinite_damage;

	// disable and restore original code
	if (!infinite_damage) {
		if (inf_dmg_info.code_cave_adr)
			inf_dmg_info.free_code_cave(proc);

		inf_dmg_info.restore_original_code(proc);
	}
	else {
		// inject code

		if (!inf_dmg_info.code_cave_adr)
			inf_dmg_info.alloc_code_cave(proc);

		// calculate the JMP offset to get back to the next line in the original code from the injected code cave
		DWORD original_code_offset = inf_dmg_info.hook_at_adr + inf_dmg_info.original_code_size();
		original_code_offset = original_code_offset - (inf_dmg_info.code_cave_adr + 13 + 5);
		inf_dmg_info.set_shellcode_dword(14, original_code_offset);

		inf_dmg_info.write_code_cave(proc);

		std::cout << "INFINITE DAMAGE CODE CAVE: " << std::hex << inf_dmg_info.code_cave_adr << '\n';

		jump_hook(proc, inf_dmg_info.hook_at_adr, inf_dmg_info.code_cave_adr, inf_dmg_info.original_code_bytes());
	}
}

void PlayerController::toggle_inf_jumping()
{
	if (!inf_jump_info.hook_at_adr) {
		inf_jump_info.hook_at_adr = get_jump_nop_address();
		if (!inf_jump_info.hook_at_adr) {
			std::cout << "INVALID INFINITE JUMP ADDRESS!\n";
			return;
		}
	}

	infinite_jumping = !infinite_jumping;
	
	if (infinite_jumping)  // enable
		nop_address(proc, inf_jump_info.hook_at_adr, inf_jump_info.original_code_size());
	else
		inf_jump_info.restore_original_code(proc);
}

void PlayerController::init_player_controller()
{
	if (!base_p)
		base_p = get_player_controller_address();

	if (!base_p) {
		std::cout << "PlayerController not found! Make sure to start a level.\n";
		return;
	}

	hp_p = base_p + 0x60;
	hp_max_p = base_p + 0x5C;

	super_invincibility_p = base_p + 0x6c;

	DWORD loadout_p = read_memory<DWORD>(proc, base_p + 0x38);
	primary_weapon_p = loadout_p + 0x8;
	secondary_weapon_p = loadout_p + 0xC;
}

/** Gets the PlayerController address using jump hooking and code injection.

1. Find the function where the hook is to be placed.
2. Create a code cave for the injected assembly code and for data variables.
3. Place the jump hook.
4. Wait for the injected code to be run.
5. Read the address stored by the injected code in the process' memory.
6. ???
7. Profit.

The injected code can't write anything to the memory in this process because it's injected
into Cuphead.exe's process and each process has its own virtual address space (VAS), so it
writes it directly into newly allocated memory in the target process.
*/
DWORD PlayerController::get_player_controller_address()
{
	static const BYTE func_header[] = {
		0x55,
		0x8B, 0xEC,
		0x83, 0xEC, 0x08,
		0x8B, 0x45, 0x08,
		0x0F, 0xB6, 0x40, 0x58,
		0x85, 0xC0,
		0x74, 0x04
	};
	// 1. Find the function where the hook is to be placed based on the function's signature.
	DWORD original_func_adr = find_signature(proc, func_header, sizeof(func_header) / sizeof(BYTE));
	if (!original_func_adr) return 0;

	InjectionHelper player_controller_info;
	player_controller_info.hook_at_adr = original_func_adr + 0x23;

	std::cout << "PLAYER CONTROLLER HOOK AT ADDRESS: " << std::hex << player_controller_info.hook_at_adr << '\n';

	player_controller_info.shellcode = {
		0xA3, 0x00, 0x00, 0x00, 0x00,				// MOV &player_controller_address, eax
		0x8B, 0xC8, 0x39, 0x09, 0x8B, 0x40, 0x60,	// original code replaced by the JMP hook
		0xE9, 0x00, 0x00, 0x00, 0x00,				// jump back to original code
		0x00, 0x00, 0x00, 0x00						// player_controller_address data (var_data_adr)
	};
	player_controller_info.original_code = {
		0x8B, 0xC8, 0x39, 0x09, 0x8B, 0x40, 0x60
	};

	// 2. allocate memory for the code cave in Cuphead.exe's virtual address space
	player_controller_info.alloc_code_cave(proc);
	// make the memory writable, otherwise "MOV &player_controller_address, eax" won't work
	// *could also allocate new memory for the player_controller_address variable and change the protection only there ...
	protect_memory<BYTE>(proc, player_controller_info.code_cave_adr, PAGE_EXECUTE_READWRITE, player_controller_info.shellcode_size());

	DWORD var_data_adr = player_controller_info.code_cave_adr + 17;
	player_controller_info.set_shellcode_dword(1, var_data_adr);

	// calculate the JMP offset to get back to the next line in the original code from the injected code cave
	DWORD original_code_offset = player_controller_info.hook_at_adr + player_controller_info.original_code_bytes();
	original_code_offset = original_code_offset - (player_controller_info.code_cave_adr + 12 + 5);
	player_controller_info.set_shellcode_dword(13, original_code_offset);

	// write the shellcode to the allocated cave
	player_controller_info.write_code_cave(proc);

	std::cout << "PLAYER CONTROLLER CODE CAVE: " << std::hex << player_controller_info.code_cave_adr << '\n';

	// 3. Place the jump hook. 
	// the first 5 bytes at hook_at will be replaced with JMP cave_adr and the next 2 bytes will be NOPs
	// (the hook must be the same size as replacement of original_code)
	jump_hook(proc, player_controller_info.hook_at_adr, player_controller_info.code_cave_adr, player_controller_info.original_code_bytes());

	// 4. wait for the injected code to be executed
	DWORD player_controller_address = 0;
	while (player_controller_address == 0)
		player_controller_address = read_memory<DWORD>(proc, var_data_adr);

	std::cout << "PLAYER CONTROLLER ADDRESS: " << std::hex << player_controller_address << '\n';

	// clean up
	player_controller_info.free_code_cave(proc);  // free the allocated code cave
	player_controller_info.restore_original_code(proc);  // reset the jump hook with the original assembly code

	return player_controller_address;
}

/* Finds the address of the instruction to NOP that allows infinite jumping. */
DWORD PlayerController::get_jump_nop_address()
{
	static const BYTE signature[] = {
		0x0F, 0x85, 0xBC, 0x01, 0x00, 0x00,
		0x8B, 0x47, 0x38, 0x83, 0xEC, 0x0C, 0x50, 0x39, 0x00
	};
	DWORD adr = find_signature(proc, signature, sizeof(signature) / sizeof(BYTE));
	std::cout << "INFINITE JUMP HOOK AT ADDRESS: " << std::hex << adr << '\n';
	return adr;
}

/** Finds the address of the instruction where the JMP hook will be placed to intercept the projectile's info and set the damage to infinity.
	The damage to a player is always 1, but to the enemy, it depends on the weapon.
*/
DWORD PlayerController::get_inf_dmg_address()
{
	static const BYTE signature[] = {
		0x8B, 0x46, 0x38, 0x83, 0xEC, 0x08, 0x57, 0x50, 0x90, 0x90, 0x90, 0xFF, 0x50, 0x0C, 0x83, 0xC4, 0x10, 0x8B, 0x46, 0x3C
	};
	DWORD adr = find_signature(proc, signature, sizeof(signature) / sizeof(BYTE));
	std::cout << "INFINITE DAMAGE HOOK AT ADDRESS: " << std::hex << adr << '\n';
	return adr;
}