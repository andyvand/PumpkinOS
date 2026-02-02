#ifdef WINDOWS
#include <windows.h>

BOOL __stdcall DllMain(HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	return (TRUE);
}
#endif

