#pragma once
#include <format>
#include <string>
#include <mutex>
#include <iostream>

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
			std::print(std::cout, "\033[0m\033[1;33m");
			std::print(std::cout, "[{}] ", name_);
			std::println(std::cout, fmt, std::forward<Args>(args)...);
			std::print(std::cout, "\033[0m");
		}

		template <typename... Args>
		void Error(const std::format_string<Args...> fmt, Args&&... args) {
			std::lock_guard guard{ lock_ };
#ifdef _DEBUG
			std::print(std::cerr, "\033[0m\033[1;31m");
			std::print(std::cerr, "[{}] ", name_);
			std::println(std::cerr, fmt, std::forward<Args>(args)...);
			std::print(std::cerr, "\033[0m");
#else
			MessageBoxA(NULL, std::format(fmt, std::forward<Args>(args)...).c_str(), "HYPR ERROR", MB_ICONERROR);
#endif

		}

	};
}
