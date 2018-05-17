#include "memory_tools.h"


/* Hacks that control Cuphead's stats during a level (eg. HP, invincibilitiy, weapon loadout, ...).*/
class PlayerControllerBot
{
public:
	static const size_t N_WEAPONS = 6;
	struct Weapon 
	{
		const char* name;
		DWORD id;
	};
	// Index to Weapon table
	static const std::array<Weapon, N_WEAPONS> WEAPON_TABLE;

	~PlayerControllerBot();

	bool init();
	void exit();

	DWORD get_base_address();

	bool set_primary_weapon(const Weapon& weapon);
	bool set_secondary_weapon(const Weapon& weapon);

	void set_invincible(bool invincible);

	void set_hp(DWORD new_hp);
	DWORD get_max_hp();

	static BasicHookInfo player_controller_hook;
	static DWORD player_controller_adr;
	static DWORD original_base_address_func;
private:
	bool set_loadout_address();

	DWORD primary_weapon_adr, secondary_weapon_adr;

	bool set_base_address_hook();
	bool restore_base_address_hook();
};