#pragma once

#include <hyprutils/singleton.h>
#include <hyprocess/process_starter.h>
#include <hyprfile/runtime_dump_file.h>


namespace hyprtrace
{
	class Tracer : public hyprutils::Singleton<Tracer>
	{
	private:

	public:
		void InitializeProcessStarter(hyprocess::ProcessStarter& starter, hyprfile::RuntimeDumpFile& runtime_dump_file);
	};
}
