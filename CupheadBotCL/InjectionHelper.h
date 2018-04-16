#pragma once
#include <vector>
#include "memory_tools.h"


struct InjectionHelper {
	DWORD code_cave_adr;
	DWORD hook_at_adr;

	std::vector<BYTE> original_code;
	std::vector<BYTE> shellcode;

	void write_code_cave(HANDLE proc) const { WriteProcessMemory(proc, (LPVOID)code_cave_adr, shellcode.data(), shellcode_bytes(), NULL); }
	DWORD alloc_code_cave(HANDLE proc) { code_cave_adr = (DWORD)(VirtualAllocEx(proc, NULL, shellcode_bytes(), MEM_COMMIT, PAGE_EXECUTE)); return code_cave_adr; }
	void free_code_cave(HANDLE proc) { VirtualFreeEx(proc, (LPVOID)code_cave_adr, sizeof(shellcode), MEM_RELEASE); code_cave_adr = 0; }

	void restore_original_code(HANDLE proc) const { write_code_buffer(proc, hook_at_adr, original_code.data(), original_code_size()); }

	void set_shellcode_dword(size_t index, DWORD value) { memcpy(&shellcode[index], &value, sizeof(DWORD)); }

	size_t original_code_size() const { return original_code.size(); }
	size_t original_code_bytes() const { return original_code.size() * sizeof(BYTE); }
	size_t shellcode_size() const { return shellcode.size(); }
	size_t shellcode_bytes() const { return shellcode.size() * sizeof(BYTE); }
};