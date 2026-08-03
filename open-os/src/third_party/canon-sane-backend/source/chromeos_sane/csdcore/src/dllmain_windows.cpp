/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <windows.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
HMODULE g_hModule = NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
		g_hModule = hModule;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
#ifdef _DEBUG
		_CrtDumpMemoryLeaks();
#endif
		break;
	}
	return TRUE;
}

