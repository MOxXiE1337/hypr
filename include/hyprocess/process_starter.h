#pragma once
#include "hyprocess.h"
#include <hyprutils/logmanager.h>

namespace hyprocess
{
	class ProcessStarter
	{
	private:
		struct MemoryReservation
		{
			uintptr_t address;
			size_t    size;
			uint32_t  protect;
		};

	private:
		hyprutils::LogManager logman_;
		std::string image_path_;
		std::string cmdline_;
		std::vector<MemoryReservation> memory_reservations_;
	public:
		ProcessStarter(const std::string& image_path = "", const std::string& cmdline = "") : logman_("ProcessStarter"), image_path_(image_path), cmdline_(cmdline) {}
		void SetImagePath(const std::string& image_path) { image_path_ = image_path; }
		void SetCommandLine(const std::string& cmdline) { cmdline_ = cmdline; }
		void ReserveMemory(uintptr_t address, size_t size, uint32_t protect);
		bool StartProcess();
	};
}
