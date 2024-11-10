#pragma once

#include <hypr/loader.h>

#include <hyprutils/singleton.h>
#include <hyprocess/process_starter.h>
#include <hyprfile/runtime_dump_file.h>


namespace hyprtrace
{
	class Tracer : public hyprutils::Singleton<Tracer>
	{
	private:
		static hypr::Loader* loader_;
		static hyprutils::LogManager logman_;

		static LONG NTAPI ExceptionHandler(struct _EXCEPTION_POINTERS* exception);
	public:
		static hyprutils::LogManager& GetLogManager() { return logman_; };

		static bool Initialize(hypr::Loader* loader);
		static bool InitializeProcessStarter(hyprocess::ProcessStarter& starter, hyprfile::RuntimeDumpFile& runtime_dump_file);
	};
}
