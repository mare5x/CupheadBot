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
	if (player_controller.initialized()) {
		player_controller.set_hard_invincibility(true);
		player_controller.set_hp(5);
		std::cout << "HP: " << player_controller.get_hp() << '\n';
		std::string input;
		while (std::cin >> input) {
			if (input == "j") {
				player_controller.toggle_inf_jumping();
				std::cout << "INFINITE JUMPING: " << (player_controller.infinite_jumping_enabled() ? "ON" : "OFF") << '\n';
			}
		}
	}

	CloseHandle(proc);

	return 0;
}