#include <Windows.h>
#include <d3d9.h>
#include <string>
#include <array>
#include <fstream>


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
DWORD protect_memory(DWORD hook_at, DWORD protection)
{
	DWORD old_protection;
	VirtualProtect((LPVOID)hook_at, sizeof(T), protection, &old_protection);
	return old_protection;
}


DWORD get_VF(DWORD class_adr, DWORD func_idx)
{
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


DWORD original_endScene_address;
std::array<BYTE, 5> originals;
LPDIRECT3DDEVICE9 discovered_device;
DWORD __stdcall on_init_EndScene(LPDIRECT3DDEVICE9 device)
{
	discovered_device = device;
	jump_unhook(original_endScene_address, originals);
	return original_endScene_address;
}


/* __declspec(naked) tells the compiler to leave the code as is. */
__declspec(naked) void EndScene_trampoline()
{
	__asm {
		MOV EAX, DWORD PTR SS:[ESP + 0x4]   // the device pointer is the first argument
		PUSH EAX							// eax contains the device pointer
		CALL on_init_EndScene				// the original endscene function address is returned in the eax register
		JMP EAX								// jmp back to the original endscene
	}
}


DWORD get_EndScene_adr()
{
	/*
	0. create a window (for the p_device)
	1. create a p_device
	2. find the func address in the VF table (class VF tables are shared by all instances in a process, 
	so the address found will be the same as the one used by the game)
	3. clean up
	*/

	WNDCLASSEXA wc =
	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		DefWindowProc,
		0L,0L,
		GetModuleHandleA(NULL),
		NULL, NULL, NULL, NULL,
		"DX", NULL
	};

	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 600, 600, GetDesktopWindow(), NULL, wc.hInstance, NULL);

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D) return 0;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;

	IDirect3DDevice9* p_device;
	HRESULT res = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &p_device);
	if (FAILED(res)) return 0;

	DWORD endScene_adr = get_VF((DWORD)p_device, 42);

	pD3D->Release();
	p_device->Release();
	DestroyWindow(hWnd);

	return endScene_adr;
}


DWORD WINAPI run_bot(LPVOID param)
{
	std::ofstream ofs("C:\\Users\\Mare5\\Desktop\\bot.log");

	original_endScene_address = get_EndScene_adr();
	originals = jump_hook(original_endScene_address, (DWORD)&EndScene_trampoline);

	while ((DWORD)discovered_device == 0) { Sleep(10); }
	ofs << std::hex << original_endScene_address << '\n' << std::hex << (DWORD)discovered_device << '\n';
	ofs.close();

	MessageBoxA(NULL, std::to_string((DWORD)discovered_device).c_str(), "TEST", MB_OK | MB_TOPMOST);

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