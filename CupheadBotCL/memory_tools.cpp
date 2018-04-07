#include <iostream>
#include <vector>
#include "memory_tools.h"


void log_error(const char* msg)
{
	std::cout << msg << ", ERROR: " << GetLastError() << '\n';
}


DWORD get_pid(const std::string& window_title)
{
	HWND wnd = FindWindow(NULL, window_title.c_str());
	if (!wnd) {
		log_error("FindWindow failed!");
		return 0;
	}
	DWORD pid;
	GetWindowThreadProcessId(wnd, &pid);
	return pid;
}


DWORD get_base_address(HANDLE proc)
{
	HMODULE k32 = GetModuleHandle("kernel32.dll");
	LPVOID func_adr = GetProcAddress(k32, "GetModuleHandleA");
	if (!func_adr)
		func_adr = GetProcAddress(k32, "GetModuleHandleW");

	HANDLE thread = CreateRemoteThread(proc, NULL, NULL, (LPTHREAD_START_ROUTINE)func_adr, NULL, NULL, NULL);
	WaitForSingleObject(thread, INFINITE);

	DWORD base;
	GetExitCodeThread(thread, &base);

	CloseHandle(thread);

	return base;
}


/* Returns the next memory page after base_adr that has the PAGE_EXECUTE_READWRITE permission. */
MemoryPage next_memory_page(HANDLE proc, DWORD base_adr)
{
	MEMORY_BASIC_INFORMATION mem_info;
	while (VirtualQueryEx(proc, (LPVOID)base_adr, &mem_info, sizeof(mem_info)) != 0) {
		if (mem_info.AllocationProtect & PAGE_EXECUTE_READWRITE) {
			printf("%x %d %x\n", mem_info.BaseAddress, mem_info.RegionSize, mem_info.AllocationProtect);
			return MemoryPage((DWORD)mem_info.BaseAddress, mem_info.RegionSize);
		}
		base_adr += mem_info.RegionSize;
	}
	return MemoryPage();
}


MemoryPage first_memory_page(HANDLE proc)
{
	return next_memory_page(proc, 0);
}


/*
Note: Cuphead uses JIT (just in time) compilation, so make sure the desired
function has been assembled in memory before running this function.

improvement idea: scan until function header -> check if pattern in function -> repeat
*/
DWORD find_function(HANDLE proc, BYTE func_header[], size_t size)
{
	std::vector<BYTE> mem_buffer(size);

	MemoryPage page = first_memory_page(proc);
	while (page.valid()) {
		for (DWORD adr = page.base_adr; adr < page.base_adr + page.size; ++adr) {
			read_memory(proc, adr, mem_buffer.data(), size);
			bool found = true;
			for (size_t i = 0; i < size; ++i) {
				if (func_header[i] != mem_buffer[i]) {
					found = false;
					break;
				}
			}
			if (found) {
				printf("found func at: %x\n", adr);
				return adr;
			}
		}

		page = next_memory_page(proc, page.base_adr + page.size + 1);
	}

	return 0;
}


/** Place a JMP instruction at hook_at address that jumps to jmp_adr.

bytes_to_replace are the number of bytes to be replaced by the new JMP instruction.
(5 bytes for the JMP and bytes_to_replace - 5 bytes for leftover instructions that wouldn't work with the new JMP ...)
*/
void jump_hook(HANDLE proc, DWORD hook_at, DWORD jmp_adr, int bytes_to_replace)
{
	if (bytes_to_replace > 12)
		return;

	DWORD new_offset = jmp_adr - hook_at - 5;  // JMP call addresses are relative to the address of the JMP ... 

	DWORD old_protection = protect_memory<DWORD[3]>(proc, hook_at, PAGE_EXECUTE_READWRITE);
	// JMP ADR = always 5 bytes
	write_memory<BYTE>(proc, hook_at, 0xE9);  // JMP
	write_memory<DWORD>(proc, hook_at + 1, new_offset);

	// replace left over instruction bytes with NOPs
	for (int i = 5; i < bytes_to_replace; ++i) {
		write_memory<BYTE>(proc, hook_at + i, 0x90);  // NOP
	}

	protect_memory<DWORD[3]>(proc, hook_at, old_protection);
}