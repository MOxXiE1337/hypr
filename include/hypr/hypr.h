#pragma once

#include <print>

#include <memory>

#include <mutex>

#include <string>
#include <format>
#include <vector>
#include <unordered_map>

#include <Windows.h>


namespace hypr
{
	// segaddr means the address in the old process
	typedef uintptr_t segaddr_t;

	inline bool InvokeStandardEntrypoint(uintptr_t address, uintptr_t imagebase)
	{
		typedef BOOL(__stdcall* DllEntryPointFn)(uintptr_t, DWORD, LPVOID);
		return reinterpret_cast<DllEntryPointFn>(address)(imagebase, DLL_PROCESS_ATTACH, nullptr) == TRUE;
	}
}

