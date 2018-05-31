#include "PlayerDataBot.h"


const std::array<PlayerDataBot::Loadout::Weapon, PlayerDataBot::Loadout::N_WEAPONS> PlayerDataBot::Loadout::WEAPON_TABLE = { {
	{ "PEASHOOTER", 1456773641 },
	{ "SPREAD", 1456773649 },
	{ "CHASER", 1460621839 },
	{ "LOBBER", 1467024095 },
	{ "CHARGE", 1466416941 },
	{ "ROUNDABOUT", 1466518900 }
} };


const std::array<PlayerDataBot::Loadout::Charm, PlayerDataBot::Loadout::N_CHARMS> PlayerDataBot::Loadout::CHARM_TABLE = { {
	{ "HEART", 0x571289E6 },
	{ "COFFEE", 0x571345E2 },
	{ "SMOKE BOMB", 0x57151B56 },
	{ "P. SUGAR", 0x58A299CC },
	{ "TWIN HEART", 0x5971F75B },
	{ "WHETSTONE", 0x5971ACAF }
} };


DWORD PlayerDataBot::get_player_data_func_adr = 0;


DWORD PlayerDataBot::get_money_address()
{
	DWORD adr = get_player_data_address();
	if (!adr) return 0;

	adr = read_memory<DWORD>(adr + 0xc);
	adr = read_memory<DWORD>(adr + 0x8);
	return adr + 0x14;
}

DWORD PlayerDataBot::get_loadout_address()
{
	DWORD adr = get_player_data_address();
	if (!adr) return 0;

	adr = read_memory<DWORD>(adr + 0x8);
	return read_memory<DWORD>(adr + 0x8);
}

bool PlayerDataBot::initialized_or_init_signature()
{
	static const BYTE signature[] = {
		0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08, 0xE8, 0x2D, 0x00, 0x00, 0x00, 0x83, 0xEC, 0x0C, 0x50, 0xE8, 0x4C, 0x00, 0x00, 0x00, 0x83, 0xC4, 0x10, 0xC9, 0xC3
	};
	if (!get_player_data_func_adr)
		get_player_data_func_adr = find_signature(signature, sizeof(signature));
	return get_player_data_func_adr;
}

DWORD PlayerDataBot::get_money()
{
	if (DWORD adr = get_money_address())
		return read_memory<DWORD>(adr);
	return 0;
}

bool PlayerDataBot::set_money(DWORD money)
{
	if (DWORD adr = get_money_address()) {
		write_memory<DWORD>(adr, money);
		return true;
	}
	return false;
}

DWORD PlayerDataBot::get_player_data_address()
{
	if (!initialized_or_init_signature())
		return 0;

	// Call a function that returns the address to the player data structure.
	// The start of the pointer chain depends on the currently loaded save file, but with this there aren't any problems.
	DWORD adr = 0;
	__asm {
		CALL[get_player_data_func_adr]
		mov adr, eax
	}
	return adr;
}