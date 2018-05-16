#include "PlayerControllerBot.h"

// static members are declared in a class and defined here https://stackoverflow.com/questions/1410563/what-is-the-difference-between-a-definition-and-a-declaration
JumpHookInfo PlayerControllerBot::player_controller_hook = {
	0,
	{ 0x8B, 0xC8, 0x39, 0x09, 0x8B, 0x40, 0x60 }
};
DWORD PlayerControllerBot::player_controller_adr = 0;
DWORD PlayerControllerBot::original_base_address_func = 0;


/** Trampoline function used to get the address of the PlayerController and then going back to the original function.
	The function hook persists from set_base_address_hook to restore_base_address_hook. That means the player controller
	address gets constantly updated.*/
void __declspec(naked) base_address_getter()
{
	__asm 
	{
		MOV [PlayerControllerBot::player_controller_adr], EAX
		mov    ecx, eax														// original code replaced by the hook
		cmp    DWORD PTR[ecx], ecx
		mov    eax, DWORD PTR[eax + 0x60]
		JMP [PlayerControllerBot::original_base_address_func]				// jump back to original code
	}
}


PlayerControllerBot::~PlayerControllerBot()
{
	exit();
}

bool PlayerControllerBot::init()
{
	return set_base_address_hook();
}

void PlayerControllerBot::exit()
{
	restore_base_address_hook();
}

void PlayerControllerBot::set_invincible(bool invincible)
{
	if (!original_base_address_func)
		init();
	if (player_controller_adr)
		write_memory<bool>(player_controller_adr + 0x6c, invincible);
}

void PlayerControllerBot::set_hp(DWORD new_hp)
{
	if (!original_base_address_func)
		init();
	if (player_controller_adr)
		write_memory<DWORD>(player_controller_adr + 0x60, new_hp);
}

DWORD PlayerControllerBot::get_max_hp()
{
	if (!original_base_address_func)
		init();
	if (player_controller_adr)
		return read_memory<DWORD>(player_controller_adr + 0x5c);
}

bool PlayerControllerBot::set_base_address_hook()
{
	static const BYTE signature[] = {
		0x8B, 0xC8, 0x39, 0x09, 0x8B, 0x40, 0x60, 0x85, 0xC0, 0x0F, 0x9F, 0xC0, 0x0F, 0xB6, 0xC0
	};

	player_controller_hook.hook_at = find_signature(signature, sizeof(signature));
	if (!player_controller_hook.hook_at) return false;

	original_base_address_func = jump_hook(player_controller_hook.hook_at, (DWORD)&base_address_getter, player_controller_hook.original_bytes.size());
	
	return true;
}

bool PlayerControllerBot::restore_base_address_hook()
{
	if (player_controller_hook.hook_at) {
		jump_unhook(player_controller_hook.hook_at, player_controller_hook.original_bytes.data(), player_controller_hook.original_bytes.size());
		player_controller_hook.hook_at = 0;
	}
	return true;
}