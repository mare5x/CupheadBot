#include <iostream>
#include "memory_tools.h"
#include "PlayerController.h"


DWORD get_wallhack_address(HANDLE proc)
{
	DWORD base = get_base_address(proc);
	// static pointer chain
	DWORD ptr0 = read_memory<DWORD>(proc, base + 0x104FA20);
	DWORD ptr1 = read_memory<DWORD>(proc, ptr0 + 4);
	DWORD cuphead_world_player_ptr = read_memory<DWORD>(proc, ptr1 + 0x44);
	return cuphead_world_player_ptr;
}


DWORD wallhack_on(HANDLE proc)
{
	DWORD adr = get_wallhack_address(proc);
	write_memory<DWORD>(proc, adr, 1);
	return adr;
}


DWORD wallhack_off(HANDLE proc)
{
	DWORD adr = get_wallhack_address(proc);
	write_memory<DWORD>(proc, adr, 2);
	return adr;
}


int main()
{
	DWORD PID = get_pid("Cuphead");
	if (!PID) return -1;

	HANDLE proc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, PID);
	if (!proc || proc == INVALID_HANDLE_VALUE) {
		log_error("Invalid handle");
		printf("Try running as admin.\n");
		return -1;
	}

	std::cout << "Base: " << std::hex << get_base_address(proc) << '\n';

	DWORD adr = get_wallhack_address(proc);
	printf("%x: %d\n", adr, read_memory<DWORD>(proc, adr));
	wallhack_on(proc);
	printf("%x: %d\n", adr, read_memory<DWORD>(proc, adr));

	PlayerController player_controller(proc);
	while (true) {
		player_controller.set_hp(3);
		std::cout << player_controller.get_hp() << '\n';
	}

	CloseHandle(proc);

	return 0;
}