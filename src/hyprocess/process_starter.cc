#include <hyprocess/process_starter.h>

namespace hyprocess
{
	void ProcessStarter::ReserveMemory(uintptr_t address, size_t size, uint32_t protect)
	{
		memory_reservations_.push_back({ address,size,protect});
	}

	void ProcessStarter::ReserveMemoryFromRuntimeDumpFile(hyprfile::RuntimeDumpFile& runtime_dump_file)
	{
		std::vector<hyprfile::RuntimeDumpFile::ModuleRecord> modules;
		runtime_dump_file.GetModuleRecords(modules);

		for (auto& module : modules)
		{
			ReserveMemory(uintptr_t(module.imagebase), module.imagesize, PAGE_EXECUTE_READWRITE);
		}
	}

	void ProcessStarter::ReserveMemoryFromSegmentsFile(hyprfile::SegmentsFile& segments_file)
	{
		std::vector<hyprfile::SegmentsFile::Segment> segments;
		segments_file.GetSegments(segments);

		for (auto& segment : segments)
		{
			ReserveMemory(segment.address, segment.vsize, PAGE_EXECUTE_READWRITE);
		}
	}

	HANDLE ProcessStarter::StartProcess()
	{
		if (image_path_.empty())
		{
			logman_.Error("image path is empty");
			return NULL;
		}

		std::filesystem::path image_path{ image_path_ };

		STARTUPINFOA si{};
		PROCESS_INFORMATION pi{};

		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = TRUE;

		BOOL success = CreateProcessA(
			image_path_.c_str(),
			const_cast<LPSTR>((image_path_ + " " + cmd_parameters_).c_str()),
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
			return NULL;
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
				TerminateProcess(pi.hProcess, -1);
				return NULL;
			}
			logman_.Log("reserved memory {:X}:{:X}", reservation.address, reservation.size);
		}

		if (ResumeThread(pi.hThread) == FALSE)
		{
			logman_.Error("failed to resume process");
			logman_.Error("last error code: {:X}", GetLastError());
			TerminateProcess(pi.hProcess, -1);
			return NULL;
		}

		logman_.Log("reserved {} memory pages", memory_reservations_.size());
		logman_.Log("process started, pid: {:X}", pi.dwProcessId);

		return pi.hProcess;
	}
}

