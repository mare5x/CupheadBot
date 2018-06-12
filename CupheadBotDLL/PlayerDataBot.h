#pragma once
#include "memory_tools.h"


class PlayerDataBot
{
public:
	struct Loadout
	{
		struct _LoadoutItem
		{
			const char* name;
			DWORD id;
		};

		static const size_t N_WEAPONS = 9;
		typedef _LoadoutItem Weapon;
		static const std::array<Weapon, N_WEAPONS> WEAPON_TABLE;  // Index to Weapon table

		static const size_t N_SUPERS = 3;
		typedef _LoadoutItem Super;
		static const std::array<Super, N_SUPERS> SUPER_TABLE;

		static const size_t N_CHARMS = 7;
		typedef _LoadoutItem Charm;
		static const std::array<Charm, N_CHARMS> CHARM_TABLE;

		Loadout(DWORD loadout_address) : loadout_adr(loadout_address) { }
		
		bool is_valid() const { return loadout_adr; }

		void set_primary_weapon(const Weapon& weapon) { write_memory<DWORD>(loadout_adr + 0x8, weapon.id); }
		void set_secondary_weapon(const Weapon& weapon) { write_memory<DWORD>(loadout_adr + 0xC, weapon.id); }
		void set_super(const Super& super) { write_memory<DWORD>(loadout_adr + 0x10, super.id); }
		void set_charm(const Charm& charm) { write_memory<DWORD>(loadout_adr + 0x14, charm.id); }

		const Weapon& get_primary_weapon() const;
		const Weapon& get_secondary_weapon() const;
		const Super& get_super() const;
		const Charm& get_charm() const;

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
typedef PlayerDataBot::Loadout::Super LoadoutSuper;


bool operator==(const PlayerDataBot::Loadout::_LoadoutItem& left, const PlayerDataBot::Loadout::_LoadoutItem& right);