#include <hyprocess/process_starter.h>

namespace hyprocess
{
	void ProcessStarter::ReserveMemory(uintptr_t address, size_t size, uint32_t protect)
	{
		memory_reservations_.push_back({ address,size,protect });
	}

	bool ProcessStarter::StartProcess()
	{
		if (image_path_.empty())
		{
			logman_.Error("image path is empty");
			return false;
		}

		std::filesystem::path image_path{ image_path_ };

		STARTUPINFOA si{};
		PROCESS_INFORMATION pi{};

		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = TRUE;

		BOOL success = CreateProcessA(
			image_path_.c_str(),
			const_cast<LPSTR>(cmdline_.c_str()),
			nullptr,
			nullptr,
			FALSE,
			CREATE_SUSPENDED,
			nullptr,
			image_path.parent_path().string().c_str(),
			&si,
			&pi
		);

		if (success == FALSE)
		{
			logman_.Error("failed to start process \"{}\"", image_path_);
			logman_.Error("last error code: {:X}", GetLastError());
			return false;
		}
		
		if (ResumeThread(pi.hThread) == FALSE)
		{
			logman_.Error("failed to resume process");
			logman_.Error("last error code: {:X}", GetLastError());
			return false;
		}

		for (auto& reservation : memory_reservations_)
		{
			LPVOID ptr = VirtualAllocEx(pi.hProcess,
				reinterpret_cast<LPVOID>(reservation.address),
				reservation.size,
				MEM_COMMIT | MEM_RESERVE,
				reservation.protect);

			if (ptr == nullptr)
			{
				logman_.Error("failed to reserver memory {:X}:{:X}", reservation.address, reservation.size);
				return false;
			}
		}

		logman_.Log("reserved {} memory pages", memory_reservations_.size());
		logman_.Log("process started, pid: {:X}", pi.dwProcessId);

		return true;
	}
}

