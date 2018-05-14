#pragma once
#include <Windows.h>
#include <array>


template<typename T>
T read_memory(DWORD address)
{
	return *((T*)address);
}


template<typename T>
void write_memory(DWORD address, T value)
{
	*((T*)address) = value;
}


template<typename T>
T* point_memory(DWORD address)
{
	return (T*(address));
}


template<typename T>
DWORD protect_memory(DWORD hook_at, DWORD protection, size_t size)
{
	DWORD old_protection;
	VirtualProtect((LPVOID)hook_at, sizeof(T) * size, protection, &old_protection);
	return old_protection;
}


template<typename T>
DWORD protect_memory(DWORD hook_at, DWORD protection)
{
	return protect_memory<T>(hook_at, protection, 1);
}


/* Returns the address of the func_idx-th function in the class vtable pointed to by class_adr. */
DWORD get_VF(DWORD class_adr, DWORD func_idx);


/** Change the address of func_idx in class_adr's vtable with new_func.
Returns the original function address replaced by new_func.
*/
DWORD hook_vtable(DWORD class_adr, DWORD func_idx, DWORD new_func);


/** Places a JMP hook at hook_at.
The original bytes replaced are returned and should be restored ASAP.
(The hook must be unhooked immediately, since the extra byte after the current function gets replaced ...)
*/
const std::array<BYTE, 5> jump_hook(DWORD hook_at, DWORD new_func);


/* Restores the JMP hook at hook_at with the original bytes returned by jump_hook. */
void jump_unhook(DWORD hook_at, const std::array<BYTE, 5>& originals);


/** Place a detour hook at hook_at. 
	Seamlessly redirect/detour execution at hook_at to go to detour. 
	At detour, execute whatever you want, then it executes the length of original
	code replaced by the JMP and jumps back to the original code.
*/
BYTE* detour_hook(DWORD hook_at, DWORD detour, size_t length);


void remove_detour_hook(DWORD hook_at, const BYTE* original, size_t length);
