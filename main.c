#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <winbase.h>

void printBits(size_t size, void const * const ptr)
{
	unsigned char* b = (unsigned char*) ptr;
	unsigned char byte;
	int i, j;
	for(i = size-1; i >= 0; i--)
	{
		for(j = 7; j >= 0; j--)
		{
			byte = (b[i] >> j) & 1;
			printf("%u", byte);
		}
	}
	printf("\n");
}

HANDLE GetProcessHandleFromName(const char* processName)
{
	printf("Searching Process: %s\n", processName);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process;
 	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);
	if(Process32First(snapshot, &process))
	{
		while(Process32Next(snapshot, &process))
		{
			if(strcmp(process.szExeFile, processName) == 0)
			{
				CloseHandle(snapshot);
				return OpenProcess(PROCESS_ALL_ACCESS, FALSE, process.th32ProcessID);
				break;
			}
		}
	}
	CloseHandle(snapshot);
	return NULL;
}

int main()
{
	printf("Looking for Game Process...\n");
	HANDLE hProcess = GetProcessHandleFromName("AC4BFSP.exe");
	if(hProcess == NULL)
	{
		printf("Game Process not found. Launching game...\n");
		system("start uplay://launch/273/0");
	  Sleep(10000);	
		hProcess = GetProcessHandleFromName("AC4BFSP.exe");
	} else printf("Game Process Found. Applying Fix\n");

	printf("Getting Mask\n");
	PDWORD_PTR processMask = malloc(sizeof(PDWORD));
	PDWORD_PTR systemMask = malloc(sizeof(PDWORD));
	ZeroMemory(processMask, sizeof(PDWORD));
	ZeroMemory(systemMask, sizeof(PDWORD));
	if(!GetProcessAffinityMask(hProcess, processMask, systemMask))
	{
		printf("Error getting Mask\n");
		return -1;
	}

	printf("Size of Mask: %llu\n", sizeof(PDWORD));
	printf("System Mask: ");
	printBits(sizeof(PDWORD), systemMask);
	printf("Process Mask: ");
	printBits(sizeof(PDWORD), processMask);

	PDWORD_PTR oneCoreMask = malloc(sizeof(PDWORD));
	*oneCoreMask = 1;
	printf("New Mask: ");
	printBits(sizeof(PDWORD), oneCoreMask);

	printf("Changing Process Mask\n");
	if(SetProcessAffinityMask(hProcess, *oneCoreMask) == 0)
	{
		printf("Error Setting Mask\n");
		return -2;
	}

	printf("Reverting Process Mask\n");
	if(SetProcessAffinityMask(hProcess, *systemMask) == 0)
	{
		printf("Error Setting Mask\n");
		return -2;
	}

	CloseHandle(hProcess);
	return 0;
}
