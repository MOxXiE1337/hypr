#pragma once

#include "hyprtrace.h"

#include <hypr/loader.h>

#include <hyprocess/process_starter.h>
#include <hyprfile/runtime_dump_file.h>

namespace hyprtrace
{
	// actually multi thread issue, but why u need to opearte after initialization??? just add a lock but im lazy
	class ApiTracer
	{
	private:
		static hypr::Loader* loader_;
		static hyprutils::LogManager logman_;

		static bool should_log_;

		static std::unordered_map<hypr::segaddr_t, uintptr_t> exception_api_hooks_;
		static std::unordered_map<hypr::segaddr_t, uintptr_t> inline_api_hooks_;

		static std::unordered_map<std::string, bool> filtered_modules_;
		static std::unordered_map<hypr::segaddr_t, bool> filtered_apis_by_address_;
		static std::unordered_map<std::string, bool> filtered_apis_by_name_;

		static LONG NTAPI ExceptionHandler(struct _EXCEPTION_POINTERS* exception);
	public:
		static hyprutils::LogManager& GetLogManager();

		static bool Intialize(hypr::Loader* loader);

		static bool AddFilteringModule(const std::string& name);
		static bool RemoveFilteringModule(const std::string& name);
		static bool AddFilteringApi(hypr::segaddr_t address);
		static bool AddFilteringApi(const std::string& name);
		static bool RemoveFilteringApi(hypr::segaddr_t address);
		static bool RemoveFilteringApi(const std::string& name);

		static bool SetApiExceptionHook(const std::string& module_name, const std::string& proc_name, void* detour);
		static bool SetApiInlineHook(const std::string& module_name, const std::string& proc_name, void* detour);
		static bool RemoveApiHook(const std::string& module_name, const std::string& proc_name);

		static void EnableTraceLogging();
		static void DisableTraceLogging();
	};
}