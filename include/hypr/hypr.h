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

	// this is designed for searching like VAD
	struct AddressRange
	{
	public:
		uintptr_t start;
		size_t    size;

		AddressRange() : start(0), size(0) {}
		AddressRange(uintptr_t address, size_t size = 0) : start(address), size(size) {}

		bool operator<(const AddressRange& p) const
		{
			return start + size < p.start;
		}

		bool operator==(const AddressRange& p) const
		{
			if (size > p.size)
				return false;

			return start >= p.start && start + size < p.start + p.size;
		}
	};

	inline bool InvokeStandardEntrypoint(uintptr_t address, uintptr_t imagebase)
	{
		typedef BOOL(__stdcall* DllEntryPointFn)(uintptr_t, DWORD, LPVOID);
		return reinterpret_cast<DllEntryPointFn>(address)(imagebase, DLL_PROCESS_ATTACH, nullptr) == TRUE;
	}
}

