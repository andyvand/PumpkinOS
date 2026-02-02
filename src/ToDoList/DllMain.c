#ifdef WINDOWS
#include <windows.h>

BOOL __stdcall DllMain(HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	BOOL	bRet;

	/* Make Lint Happy */
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
#ifdef	DEBUG
		DEBUGREGISTER(hDLL);
#endif
		bRet = DisableThreadLibraryCalls((HMODULE)hDLL);
		break;
	case DLL_PROCESS_DETACH:
		break;
	default:
		break;
	}

	dwReason = dwReason;
	lpReserved = lpReserved;

	return (TRUE);
}
#endif

