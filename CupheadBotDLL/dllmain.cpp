#include <Windows.h>


DWORD WINAPI run_bot(LPVOID param)
{
	MessageBoxA(NULL, "TEST", "TEST", MB_OK | MB_TOPMOST);

	return 1;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		HANDLE thread = CreateThread(NULL, NULL, &run_bot, NULL, NULL, NULL);
		CloseHandle(thread);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) { }
    return TRUE;
}