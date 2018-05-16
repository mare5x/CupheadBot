#include "CupheadBot.h"


CupheadBot::CupheadBot(HMODULE dll_handle)
	:dll_module(dll_handle)
{
	cuphead_window_handle = FindWindow(NULL, L"Cuphead");
	cuphead_module = GetModuleHandle(NULL);
}

void CupheadBot::wallhack(bool enable)
{
	DWORD ptr_chain = read_memory<DWORD>((DWORD)cuphead_module + 0x104FA20);
	ptr_chain = read_memory<DWORD>(ptr_chain + 4);
	ptr_chain = read_memory<DWORD>(ptr_chain + 0x44);
	write_memory<DWORD>(ptr_chain, enable ? 1 : 2);
}
