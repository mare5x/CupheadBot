#include "memory_tools.h"
#include <vector>


struct JumpHookInfo
{
	DWORD hook_at;
	std::vector<BYTE> original_bytes;
};


class PlayerControllerBot
{
public:
	~PlayerControllerBot();

	bool init();
	void exit();

	void set_invincible(bool invincible);

	void set_hp(DWORD new_hp);
	DWORD get_max_hp();

	static JumpHookInfo player_controller_hook;
	static DWORD player_controller_adr;
	static DWORD original_base_address_func;
private:
	bool set_base_address_hook();
	bool restore_base_address_hook();
};