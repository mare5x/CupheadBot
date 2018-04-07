#pragma once

#include <Windows.h>
#include <string>


struct MemoryPage {
	MemoryPage() : base_adr(0), size(0) {}
	MemoryPage(DWORD _base_adr, SIZE_T _size) : base_adr(_base_adr), size(_size) {}

	DWORD base_adr;
	SIZE_T size;

	bool valid() { return size > 0; }
};


void log_error(const char* msg);


DWORD get_pid(const std::string& window_title);


DWORD get_base_address(HANDLE proc);


template<typename T>
T read_memory(HANDLE proc, DWORD address)
{
	T val;
	ReadProcessMemory(proc, (LPVOID)address, &val, sizeof(T), NULL);
	return val;
}


template<typename T>
void read_memory(HANDLE proc, DWORD address, T* buffer, size_t size)
{
	ReadProcessMemory(proc, (LPVOID)address, buffer, sizeof(T) * size, NULL);
}


template<typename T>
void write_memory(HANDLE proc, DWORD address, T val)
{
	WriteProcessMemory(proc, (LPVOID)address, &val, sizeof(T), NULL);
}


template<typename T>
DWORD protect_memory(HANDLE proc, DWORD address, DWORD protection)
{
	DWORD old_protection;
	VirtualProtectEx(proc, (LPVOID)address, sizeof(T), protection, &old_protection);
	return old_protection;
}


MemoryPage first_memory_page(HANDLE proc);

/* Returns the next memory page after base_adr that has the PAGE_EXECUTE_READWRITE permission. */
MemoryPage next_memory_page(HANDLE proc, DWORD base_adr);


DWORD find_function(HANDLE proc, BYTE func_header[], size_t size);


/** Place a JMP instruction at hook_at address that jumps to jmp_adr.

bytes_to_replace are the number of bytes to be replaced by the new JMP instruction.
(5 bytes for the JMP and bytes_to_replace - 5 bytes for leftover instructions that wouldn't work with the new JMP ...)
*/
void jump_hook(HANDLE proc, DWORD hook_at, DWORD jmp_adr, int bytes_to_replace);