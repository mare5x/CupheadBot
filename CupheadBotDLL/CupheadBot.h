#pragma once
#include "memory_tools.h"

class CupheadBot
{
public:
	CupheadBot(HMODULE dll_handle);

	void wallhack(bool enable);

	HMODULE get_dll_module() const { return dll_module; }
	HMODULE get_cuphead_module() const { return cuphead_module; }
	HWND get_cuphead_window_handle() const { return cuphead_window_handle; }
private:
	HMODULE dll_module;
	HMODULE cuphead_module;
	HWND cuphead_window_handle;
};