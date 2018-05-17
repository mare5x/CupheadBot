#include "memory_tools.h"


/* Hacks that control Cuphead's stats during a level (eg. HP, invincibilitiy, weapon loadout, ...).*/
class PlayerControllerBot
{
public:
	~PlayerControllerBot();

	bool init();
	void exit();

	void set_invincible(bool invincible);

	void set_hp(DWORD new_hp);
	DWORD get_max_hp();

	static BasicHookInfo player_controller_hook;
	static DWORD player_controller_adr;
	static DWORD original_base_address_func;
private:
	bool set_base_address_hook();
	bool restore_base_address_hook();
};