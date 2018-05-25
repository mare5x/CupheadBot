#include "memory_tools.h"


/* Hacks that control Cuphead's stats during a level (eg. HP, invincibilitiy, weapon loadout, ...). */
class PlayerControllerBot
{
public:
	static const float DEFAULT_SUPER_COST;

	static const size_t N_WEAPONS = 6;
	struct Weapon 
	{
		const char* name;
		DWORD id;
	};
	// Index to Weapon table
	static const std::array<Weapon, N_WEAPONS> WEAPON_TABLE;

	~PlayerControllerBot() { exit(); }

	bool init();
	void exit();

	bool set_primary_weapon(const Weapon& weapon);
	bool set_secondary_weapon(const Weapon& weapon);

	/* Setting this flag makes you unable to use super and parry. Use CupheadBot.set_invincible instead. */
	bool set_invincible(bool invincible);

	bool set_hp(DWORD new_hp);
	DWORD get_max_hp();  // Returns 0 on failure

	bool set_super_cost(float new_cost);
	bool set_super_meter(float val);
	float get_max_super();

	DWORD get_stats_address() const { return player_stats_adr; }
	DWORD get_player_controller_address() const { return player_controller_adr; }
	DWORD get_weapon_manager_address() const { return weapon_manager_adr; }

	static BasicHookInfo player_controller_hook;
	static DWORD player_stats_adr;
	static DWORD player_controller_adr;
	static DWORD weapon_manager_adr;
	static DWORD original_base_address_func;
private:
	/* Returns whether PlayerController is initialized and if it isn't it tries to initialize it. */
	bool initialized_or_init();

	bool set_addresses();

	bool set_base_address_hook();
	bool restore_base_address_hook();

	DWORD primary_weapon_adr, secondary_weapon_adr;
};