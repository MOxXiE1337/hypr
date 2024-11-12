#pragma once
#include "hyprocess.h"
#include <hyprutils/logmanager.h>
#include <hyprfile/runtime_dump_file.h>
#include <hyprfile/segments_file.h>

namespace hyprocess
{
	class ProcessStarter
	{
	private:
		struct MemoryReservation
		{
			uintptr_t   address;
			size_t      size;
			uint32_t    protect;
		};

	private:
		hyprutils::LogManager logman_;
		std::string image_path_;
		std::string cmd_parameters_;
		std::vector<MemoryReservation> memory_reservations_;
	public:
		ProcessStarter(const std::string& image_path = "", const std::string& cmd_parameters = "") : logman_("ProcessStarter"), image_path_(image_path), cmd_parameters_(cmd_parameters) {}
		
		hyprutils::LogManager& GetLogManager() { return logman_; }
		void SetImagePath(const std::string& image_path) { image_path_ = image_path; }
		void SetCommandLineParameters(const std::string& cmd_parameters) { cmd_parameters_ = cmd_parameters; }
		void ReserveMemory(uintptr_t address, size_t size, uint32_t protect);
		void ReserveMemoryFromRuntimeDumpFile(hyprfile::RuntimeDumpFile& runtime_dump_file);
		void ReserveMemoryFromSegmentsFile(hyprfile::SegmentsFile& segments_file);
		HANDLE StartProcess();
	};
}
