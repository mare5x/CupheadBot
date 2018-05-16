#pragma once

#include <Windows.h>
#include <string>


struct MemoryRegion {
	MemoryRegion() : base_adr(0), size(0) {}
	MemoryRegion(DWORD _base_adr, SIZE_T _size) : base_adr(_base_adr), size(_size) {}

	DWORD base_adr;
	SIZE_T size;

	bool valid() { return size > 0; }

	DWORD end() { return base_adr + size; }
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
void write_memory(HANDLE proc, DWORD address, const T* buffer, size_t size)
{
	WriteProcessMemory(proc, (LPVOID)(address), buffer, sizeof(T) * size, NULL);
}


template<typename T>
DWORD protect_memory(HANDLE proc, DWORD address, DWORD protection)
{
	DWORD old_protection;
	VirtualProtectEx(proc, (LPVOID)address, sizeof(T), protection, &old_protection);
	return old_protection;
}


template<typename T>
DWORD protect_memory(HANDLE proc, DWORD address, DWORD protection, size_t size)
{
	DWORD old_protection;
	VirtualProtectEx(proc, (LPVOID)address, sizeof(T) * size, protection, &old_protection);
	return old_protection;
}


MemoryRegion first_memory_page(HANDLE proc);

/* Returns the next memory page after base_adr that has the PAGE_EXECUTE_READWRITE permission. */
MemoryRegion next_memory_page(HANDLE proc, DWORD base_adr);


DWORD find_signature(HANDLE proc, const BYTE signature[], size_t size);


/** Place a JMP instruction at hook_at address that jumps to jmp_adr.

bytes_to_replace are the number of bytes to be replaced by the new JMP instruction.
(5 bytes for the JMP and bytes_to_replace - 5 bytes for leftover instructions that wouldn't work with the new JMP ...)
*/
void jump_hook(HANDLE proc, DWORD hook_at, DWORD jmp_adr, int bytes_to_replace);


void nop_address(HANDLE proc, DWORD nop_at, size_t bytes_to_replace);


/* Writes the buffer of instruction bytes to the given address. */
void write_code_buffer(HANDLE proc, DWORD address, const BYTE* buffer, size_t size);


/** Injects a dll_path DLL into the proc and returns the handle to the dll. 
	The handle is "owned" by the target process (it points to the base address of the loaded dll). 
*/
HMODULE load_dll(HANDLE proc, const wchar_t* dll_path);


void unload_dll(HANDLE proc, HMODULE dll_handle);