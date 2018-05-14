#include "memory_tools.h"


DWORD get_VF(DWORD class_adr, DWORD func_idx)
{
	// each (virtual) class has a vtable (table of addresses to functions)
	// a vtable is shared by all instances of the same class
	DWORD vtable = read_memory<DWORD>(class_adr);
	DWORD hook_adr = vtable + func_idx * sizeof(DWORD);
	return read_memory<DWORD>(hook_adr);
}


/** Change the address of func_idx in class_adr's vtable with new_func.
Returns the original function address replaced by new_func.
*/
DWORD hook_vtable(DWORD class_adr, DWORD func_idx, DWORD new_func)
{
	DWORD vtable = read_memory<DWORD>(class_adr);
	DWORD hook_at = vtable + func_idx * sizeof(DWORD);

	DWORD old_protection = protect_memory<DWORD>(hook_at, PAGE_READWRITE);
	DWORD original_func = read_memory<DWORD>(hook_at);
	write_memory<DWORD>(hook_at, new_func);
	protect_memory<DWORD>(hook_at, old_protection);

	return original_func;
}


/** Places a JMP hook at hook_at.
The original bytes replaced are returned and should be restored ASAP.
(The hook must be unhooked immediately, since the extra byte after the current function gets replaced ...)
*/
const std::array<BYTE, 5> jump_hook(DWORD hook_at, DWORD new_func)
{
	DWORD old_protection = protect_memory<BYTE[5]>(hook_at, PAGE_EXECUTE_READWRITE);

	std::array<BYTE, 5> originals;
	for (size_t i = 0; i < 5; ++i)
		originals[i] = read_memory<BYTE>(hook_at + i);

	DWORD new_offset = new_func - hook_at - 5;
	write_memory<BYTE>(hook_at, 0xE9);  // JMP
	write_memory<DWORD>(hook_at + 1, new_offset);

	protect_memory<BYTE[5]>(hook_at, old_protection);

	return originals;
}


/* Restores the JMP hook at hook_at with the original bytes returned by jump_hook. */
void jump_unhook(DWORD hook_at, const std::array<BYTE, 5>& originals)
{
	DWORD old_protection = protect_memory<BYTE[5]>(hook_at, PAGE_EXECUTE_READWRITE);

	for (size_t i = 0; i < 5; ++i)
		write_memory<BYTE>(hook_at + i, originals[i]);

	protect_memory<BYTE[5]>(hook_at, old_protection);
}


/* Delete[] the returned buffer. */
BYTE* detour_hook(DWORD hook_at, DWORD detour, size_t length)
{
	BYTE* post_detour_cave = new BYTE[length + 5];	 // a code cave directly in the target process' memory
	memcpy(post_detour_cave, (BYTE*)hook_at, length);  // copy original code
	post_detour_cave[length] = 0xE9;					 // add JMP back to original code (hook_at + ...)
	*(DWORD*)(post_detour_cave + length + 1) = (hook_at + length) - ((DWORD)(post_detour_cave)+length + 5);

	DWORD old_protection = protect_memory<BYTE[5]>(hook_at, PAGE_EXECUTE_READWRITE);
	write_memory<BYTE>(hook_at, 0xE9);  // JMP hook to detour from hook_at
	write_memory<DWORD>(hook_at + 1, detour - hook_at - 5);
	protect_memory<BYTE[5]>(hook_at, old_protection);

	DWORD _old_prot;  // make the code cave executable
	VirtualProtect(post_detour_cave, length + 5, PAGE_EXECUTE_READWRITE, &_old_prot);

	return post_detour_cave;
}


void remove_detour_hook(DWORD hook_at, const BYTE* original, size_t length)
{
	DWORD old_protection = protect_memory<BYTE>(hook_at, PAGE_EXECUTE_READWRITE, length);

	memcpy((void*)hook_at, original, length);

	protect_memory<BYTE>(hook_at, old_protection, length);
}
