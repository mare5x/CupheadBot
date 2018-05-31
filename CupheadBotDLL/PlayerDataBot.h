#pragma once
#include "memory_tools.h"


class PlayerDataBot
{
public:
	struct Loadout
	{
		static const size_t N_WEAPONS = 6;
		struct Weapon
		{
			const char* name;
			DWORD id;
		};
		// Index to Weapon table
		static const std::array<Weapon, N_WEAPONS> WEAPON_TABLE;

		static const size_t N_CHARMS = 6;
		struct Charm
		{
			const char* name;
			DWORD id;
		};
		static const std::array<Charm, N_CHARMS> CHARM_TABLE;

		Loadout(DWORD loadout_address) : loadout_adr(loadout_address) { }
		
		bool is_valid() const { return loadout_adr; }

		void set_primary_weapon(const Weapon& weapon) { write_memory<DWORD>(loadout_adr + 0x8, weapon.id); }
		void set_secondary_weapon(const Weapon& weapon) { write_memory<DWORD>(loadout_adr + 0xC, weapon.id); }
		void set_charm(const Charm& charm) { write_memory<DWORD>(loadout_adr + 0x14, charm.id); }
	private:
		DWORD loadout_adr;
	};

	bool initialized_or_init_signature();

	Loadout get_loadout() { return Loadout(get_loadout_address()); }
	
	DWORD get_money();
	bool set_money(DWORD money);

	DWORD get_player_data_address();
	DWORD get_loadout_address();
private:
	DWORD get_money_address();

	static DWORD get_player_data_func_adr;
};


typedef PlayerDataBot::Loadout::Weapon LoadoutWeapon;
typedef PlayerDataBot::Loadout::Charm LoadoutCharm;