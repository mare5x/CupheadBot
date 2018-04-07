#include <iostream>
#include "PlayerController.h"


PlayerController::PlayerController(HANDLE process, DWORD _base_p)
	: proc(process), base_p(_base_p)
{
	if (!base_p)
		base_p = get_player_controller_address();

	hp_p = base_p + 0x60;
	hp_max_p = base_p + 0x5C;
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
	BYTE func_header[] = {
		0x55,
		0x8B, 0xEC,
		0x83, 0xEC, 0x08,
		0x8B, 0x45, 0x08,
		0x0F, 0xB6, 0x40, 0x58,
		0x85, 0xC0,
		0x74, 0x04
	};
	// 1. Find the function where the hook is to be placed based on the function's signature.
	DWORD original_func_adr = find_function(proc, func_header, sizeof(func_header) / sizeof(BYTE));
	if (!original_func_adr) return 0;

	DWORD hook_at = original_func_adr + 0x23;

	BYTE shellcode[] = {
		0xA3, 0x00, 0x00, 0x00, 0x00,				// MOV &player_controller_address, eax
		0x8B, 0xC8, 0x39, 0x09, 0x8B, 0x40, 0x60,	// original code replaced by the JMP hook
		0xE9, 0x00, 0x00, 0x00, 0x00,				// jump back to original code
		0x00, 0x00, 0x00, 0x00						// player_controller_address data (var_data_adr)
	};
	BYTE original_code[] = {
		0x8B, 0xC8, 0x39, 0x09, 0x8B, 0x40, 0x60
	};
	const size_t original_code_size = sizeof(original_code) / sizeof(BYTE);

	// 2. allocate memory for the code cave in Cuphead.exe's virtual address space
	LPVOID cave_adr = VirtualAllocEx(proc, NULL, sizeof(shellcode), MEM_COMMIT, PAGE_EXECUTE);
	// make the memory writable, otherwise "MOV &player_controller_address, eax" won't work
	// *could also allocate new memory for the player_controller_address variable and change the protection only there ...
	protect_memory<BYTE[21]>(proc, (DWORD)cave_adr, PAGE_EXECUTE_READWRITE);

	DWORD var_data_adr = (DWORD)(cave_adr)+17;
	memcpy(&shellcode[1], &var_data_adr, sizeof(DWORD));

	// calculate the JMP offset to get back to the next line in the original code from the injected code cave
	DWORD original_code_offset = hook_at + original_code_size;
	original_code_offset = original_code_offset - ((DWORD)(cave_adr)+12 + 5);
	memcpy(&shellcode[13], &original_code_offset, sizeof(DWORD));

	// write the shellcode to the allocated cave
	WriteProcessMemory(proc, cave_adr, shellcode, sizeof(shellcode), NULL);

	std::cout << "cave_adr: " << std::hex << cave_adr << '\n';

	// 3. Place the jump hook. 
	// the first 5 bytes at hook_at will be replaced with JMP cave_adr and the next 2 bytes will be NOPs
	// (the hook must be the same size as replacement of original_code)
	jump_hook(proc, hook_at, (DWORD)cave_adr, original_code_size);

	// 4. wait for the injected code to be executed
	DWORD player_controller_address = 0;
	while (player_controller_address == 0)
		player_controller_address = read_memory<DWORD>(proc, var_data_adr);

	std::cout << "PLAYER CONTROLLER ADDRESS: " << std::hex << player_controller_address << '\n';

	// clean up
	VirtualFreeEx(proc, cave_adr, sizeof(shellcode), MEM_RELEASE);  // free the allocated code cave
	DWORD old_protection = protect_memory<BYTE[original_code_size]>(proc, hook_at, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(proc, (LPVOID)(hook_at), &original_code, sizeof(original_code), NULL);  // reset the jump hook with the original assembly code
	protect_memory<BYTE[original_code_size]>(proc, hook_at, old_protection);

	return player_controller_address;
}