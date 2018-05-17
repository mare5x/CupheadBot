#include "CupheadBot.h"


CupheadBot::CupheadBot(HMODULE dll_handle)
	:dll_module(dll_handle)
{
	cuphead_window_handle = FindWindow(NULL, L"Cuphead");
	cuphead_module = GetModuleHandle(NULL);

	player_controller.init();

	infinite_jump_info.original_bytes = {
		0x0F, 0x85, 0xBC, 0x01, 0x00, 0x00
	};
}

void CupheadBot::exit()
{
	set_infinite_jumping(false);
	player_controller.exit();
}

void CupheadBot::wallhack(bool enable)
{
	DWORD ptr_chain = read_memory<DWORD>((DWORD)cuphead_module + 0x104FA20);
	ptr_chain = read_memory<DWORD>(ptr_chain + 4);
	ptr_chain = read_memory<DWORD>(ptr_chain + 0x44);
	write_memory<DWORD>(ptr_chain, enable ? 1 : 2);
}

void CupheadBot::set_infinite_jumping(bool inf_jump)
{
	// get address to NOP
	if (!infinite_jump_info.hook_at) {
		infinite_jump_info.hook_at = get_infinite_jumping_address();
		if (!infinite_jump_info.hook_at)
			return;
	}

	if (inf_jump) {
		// nop address
		nop_fill(infinite_jump_info.hook_at, infinite_jump_info.original_bytes.size());
	}
	else {
		write_code_buffer(infinite_jump_info.hook_at, infinite_jump_info.original_bytes.data(), infinite_jump_info.original_bytes.size());
	}
}

DWORD CupheadBot::get_infinite_jumping_address()
{
	static const BYTE signature[] = {
		0x0F, 0x85, 0xBC, 0x01, 0x00, 0x00,
		0x8B, 0x47, 0x38, 0x83, 0xEC, 0x0C, 0x50, 0x39, 0x00
	};
	return find_signature(signature, sizeof(signature));
}
