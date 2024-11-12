#pragma once
#include "hyprutils.h"

namespace hyprutils
{
	class LogManager
	{
	private:
		std::string name_;
		bool        should_log_;
		std::mutex  lock_;
	public:
		LogManager() = delete;
		LogManager(const std::string& name) : name_(name), should_log_(true) {}

		void EnableLogging() { should_log_ = true; }
		void DisableLogging() { should_log_ = false; }

		template <typename... Args>
		void Log(const std::format_string<Args...> fmt, Args&&... args) {
			std::lock_guard guard{ lock_ };
			if (!should_log_)
				return;
			std::print(std::cout, "[{}] ", name_);
			std::println(std::cout, fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		void Warn(const std::format_string<Args...> fmt, Args&&... args) {
			std::lock_guard guard{ lock_ };
			if (!should_log_)
				return;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
			std::print(std::cout, "[{}] ", name_);
			std::println(std::cout, fmt, std::forward<Args>(args)...);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
		}

		template <typename... Args>
		void Error(const std::format_string<Args...> fmt, Args&&... args) {
			std::lock_guard guard{ lock_ };
			SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), 12);

			std::print(std::cerr, "[{}] ", name_);
			std::println(std::cerr, fmt, std::forward<Args>(args)...);

#ifdef NDEBUG
			MessageBoxA(NULL, std::format(fmt, std::forward<Args>(args)...).c_str(), "HYPR ERROR", MB_ICONERROR);
#endif
			SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), 0x07);
		}

	};
}
