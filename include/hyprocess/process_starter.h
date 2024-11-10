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
			uintptr_t   address;
			size_t      size;
			uint32_t    protect;
			std::string comment;
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
		void ReserveMemory(uintptr_t address, size_t size, uint32_t protect, const std::string& comment = "");
		bool StartProcess();
	};
}
