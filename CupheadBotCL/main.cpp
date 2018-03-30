#include <iostream>
#include <Windows.h>


void log_error(const char* msg)
{
	printf("%s, ERROR: %d\n", msg, GetLastError());
}


DWORD get_pid()
{
	HWND wnd = FindWindow(NULL, "Cuphead");
	if (!wnd) {
		log_error("FindWindow failed!");
		return 0;
	}
	DWORD pid;
	GetWindowThreadProcessId(wnd, &pid);
	return pid;
}


template<class T>
T read_memory(HANDLE proc, LPVOID address)
{
	T val;
	ReadProcessMemory(proc, address, &val, sizeof(T), NULL);
	return val;
}


template<class T>
void write_memory(HANDLE proc, LPVOID address, T val)
{
	WriteProcessMemory(proc, address, &val, sizeof(T), NULL);
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


DWORD get_wallhack_address(HANDLE proc)
{
	DWORD base = get_base_address(proc);
	// static pointer chain
	DWORD ptr0 = read_memory<DWORD>(proc, (LPVOID)(base + 0x104FA20));
	DWORD ptr1 = read_memory<DWORD>(proc, (LPVOID)(ptr0 + 4));
	DWORD cuphead_world_player_ptr = read_memory<DWORD>(proc, (LPVOID)(ptr1 + 0x44));
	return cuphead_world_player_ptr;
}


int main()
{
	DWORD PID = get_pid();
	if (!PID) return -1;

	HANDLE proc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, PID);
	if (!proc || proc == INVALID_HANDLE_VALUE) {
		log_error("Invalid handle");
		printf("Try running as admin.\n");
		return -1;
	}

	DWORD adr = get_wallhack_address(proc);
	printf("%x: %d\n", adr, read_memory<DWORD>(proc, (LPVOID)adr));
	write_memory<DWORD>(proc, (LPVOID)adr, 2);
	printf("%x: %d\n", adr, read_memory<DWORD>(proc, (LPVOID)adr));

	CloseHandle(proc);

	return 0;
}