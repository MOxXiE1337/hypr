#include <thread>

#include "hack/hack.h"

BOOL __stdcall DllMain([[maybe_unused]] HMODULE hModule, DWORD ulReason, [[maybe_unused]] LPVOID lpReserved)
{
	if (ulReason != DLL_PROCESS_ATTACH) {
		return 0;
	}

	static auto start = [&]()
		{
			hack::GetInstance().Load();
		};

	std::thread thread{ start };
	thread.detach();
	return 1;
}