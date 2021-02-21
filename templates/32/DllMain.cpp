#include <Windows.h>
#include <string>

#define FUNCTION_COUNT $FUNCTION_COUNT$

#ifdef __EDG__
#define DLLEXPORT
#else
#define DLLEXPORT(i, name) __pragma(comment(linker, "/EXPORT:" name "=" __FUNCDNAME__ ",@" #i))
#endif

extern "C" uintptr_t functions[FUNCTION_COUNT] = { 0 };
std::string functionNames[] =
{
	$FUNCTION_NAMES$
};

$FUNCTION_DEFINES$

BOOL WINAPI DllMain(HINSTANCE hinstModule, DWORD dwReason, LPVOID lpvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstModule);
		auto moduleHandle = LoadLibraryA($ORIGINAL_DLL_NAME$);
		if (moduleHandle != nullptr)
		{
			for (auto i = 0; i < FUNCTION_COUNT; i++)
			{
				auto address = GetProcAddress(moduleHandle, functionNames[i].c_str());
				functions[i] = reinterpret_cast<uintptr_t>(address);
			}
		}
	}

	return TRUE;
}
