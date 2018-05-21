#include "CupheadBot.h"


DWORD CupheadBot::original_infinite_damage_func = 0;
DWORD CupheadBot::money_function_adr = 0;


CupheadBot::CupheadBot(HMODULE dll_handle)
	:dll_module(dll_handle)
{
	cuphead_window_handle = FindWindow(NULL, L"Cuphead");
	cuphead_module = GetModuleHandle(NULL);

	player_controller.init();

	infinite_jump_info.original_bytes = { 0x0F, 0x85, 0xBC, 0x01, 0x00, 0x00 };
	infinite_parry_info.original_bytes = { 0x8B, 0x47, 0x58, 0xC7, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x47, 0x4C, 0xC7, 0x40, 0x08, 0x01, 0x00, 0x00, 0x00 };
	infinite_damage_info.original_bytes = { 0x8B, 0x46, 0x38, 0x83, 0xEC, 0x08 };
}

void CupheadBot::exit()
{
	set_infinite_jumping(false);
	set_infinite_damage(false);
	set_invincible(false);
	player_controller.exit();
}

void CupheadBot::wallhack(bool enable)
{
	DWORD ptr_chain = read_memory<DWORD>((DWORD)cuphead_module + 0x104FA20);
	ptr_chain = read_memory<DWORD>(ptr_chain + 4);
	ptr_chain = read_memory<DWORD>(ptr_chain + 0x44);
	write_memory<DWORD>(ptr_chain, enable ? 1 : 2);
}

DWORD CupheadBot::get_money()
{
	if (DWORD adr = get_money_address())
		return read_memory<DWORD>(adr);
	return 0;
}

bool CupheadBot::set_money(DWORD money)
{
	if (DWORD adr = get_money_address()) {
		write_memory<DWORD>(adr, money);
		return true;
	}
	return false;
}

bool CupheadBot::set_infinite_jumping(bool inf_jump)
{
	// get address to NOP
	if (!infinite_jump_info.hook_at) {
		infinite_jump_info.hook_at = get_infinite_jumping_address();
		if (!infinite_jump_info.hook_at)
			return false;
	}

	if (inf_jump) {
		// nop address
		nop_fill(infinite_jump_info.hook_at, infinite_jump_info.original_bytes.size());
	}
	else {
		write_code_buffer(infinite_jump_info.hook_at, infinite_jump_info.original_bytes.data(), infinite_jump_info.original_bytes.size());
	}
	return set_infinite_parry(inf_jump);
}

/* Trampoline function that sets the projectile damage to infinity (9999) and jumps back to the original code. */
void __declspec(naked) infinite_damage_setter()
{
	__asm {
		mov    DWORD PTR[edi+0x8], 0x461c3c00
		mov    eax, DWORD PTR[esi + 0x38]							// code replaced by the JMP hook
		sub    esp, 0x8
		JMP [CupheadBot::original_infinite_damage_func]				// jump back to original code
	}
}

bool CupheadBot::set_infinite_damage(bool inf_dmg)
{
	if (!infinite_damage_info.hook_at) {
		infinite_damage_info.hook_at = get_infinite_damage_address();
		if (!infinite_damage_info.hook_at)
			return false;
	}
	
	if (inf_dmg)
		CupheadBot::original_infinite_damage_func = jump_hook(infinite_damage_info.hook_at, 
															  (DWORD)&infinite_damage_setter, 
															  infinite_damage_info.original_bytes.size());
	else
		jump_unhook(infinite_damage_info.hook_at, infinite_damage_info.original_bytes.data(), infinite_damage_info.original_bytes.size());
	return true;
}

/* Note: the player must receive damage for the code to get JITted. */
bool CupheadBot::set_invincible(bool invincible)
{
	if (!invincible_adr) {
		invincible_adr = get_invincible_address();
		if (!invincible_adr)
			return false;
	}
	// The difference in invincibility is a single byte -> JNE (0x85) to JE (0x84)
	// in the damage receiver function.
	if (invincible) {
		write_memory<BYTE>(invincible_adr + 1, 0x84);
	}
	else {
		write_memory<BYTE>(invincible_adr + 1, 0x85);
	}
	return true;
}

bool CupheadBot::set_infinite_parry(bool inf_parry)
{
	if (!infinite_parry_info.hook_at) {
		infinite_parry_info.hook_at = get_infinite_parry_address();
		if (!infinite_parry_info.hook_at)
			return false;
	}

	if (inf_parry) {
		nop_fill(infinite_parry_info.hook_at, infinite_parry_info.original_bytes.size());
	}
	else {
		write_code_buffer(infinite_parry_info.hook_at, infinite_parry_info.original_bytes.data(), infinite_parry_info.original_bytes.size());
	}
	return true;
}

DWORD CupheadBot::get_infinite_jumping_address()
{
	static const BYTE signature[] = {
		0x0F, 0x85, 0xBC, 0x01, 0x00, 0x00,
		0x8B, 0x47, 0x38, 0x83, 0xEC, 0x0C, 0x50, 0x39, 0x00
	};
	return find_signature(signature, sizeof(signature));
}

DWORD CupheadBot::get_infinite_parry_address()
{
	static const BYTE signature[] = {
		0x8B, 0x47, 0x58, 0xC7, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x47, 0x4C, 0xC7, 0x40, 0x08, 0x01, 0x00, 0x00, 0x00, 
		0x8B, 0x47, 0x48, 0x83, 0xEC, 0x0C, 0x50, 0x39, 0x00, 0xE8
	};
	return find_signature(signature, sizeof(signature));
}

DWORD CupheadBot::get_infinite_damage_address()
{
	static const BYTE signature[] = {
		0x8B, 0x46, 0x38, 0x83, 0xEC, 0x08, 0x57, 0x50, 0x90, 0x90, 0x90, 0xFF, 0x50, 0x0C, 0x83, 0xC4, 0x10, 0x8B, 0x46, 0x3C
	};
	return find_signature(signature, sizeof(signature));
}

DWORD CupheadBot::get_money_address()
{
	static const BYTE signature[] = {
		0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08, 0xE8, 0x2D, 0x00, 0x00, 0x00, 0x83, 0xEC, 0x0C, 0x50, 0xE8, 0x4C, 0x00, 0x00, 0x00, 0x83, 0xC4, 0x10, 0xC9, 0xC3
	};
	if (!money_function_adr) {
		money_function_adr = find_signature(signature, sizeof(signature));
		if (!money_function_adr) return 0;
	}
	// Call a function that returns the correct address to the start of the money address pointer chain.
	// The start of the pointer chain depends on the currently loaded save file, but with this there aren't any problems.
	DWORD adr = 0;
	__asm {
		CALL [money_function_adr]
		mov adr, eax
	}

	adr = read_memory<DWORD>(adr + 0xc);
	adr = read_memory<DWORD>(adr + 0x8);
	return adr + 0x14;
}

DWORD CupheadBot::get_invincible_address()
{
	static const BYTE signature[] = {
		0x0F, 0x85, 0xE5, 0x00, 0x00, 0x00, 0x39, 0x3F, 0xD9, 0x47, 0x08, 0xD9, 0x5D, 0xF0, 0xD9, 0x45, 0xF0, 0xD9, 0xEE, 0xDF, 0xF1
	};
	return find_signature(signature, sizeof(signature));
}
