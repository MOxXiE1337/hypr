#include <hyprtrace/tracer.h> 

namespace hyprtrace
{
	void Tracer::InitializeProcessStarter(hyprocess::ProcessStarter& starter, hyprfile::RuntimeDumpFile& runtime_dump_file)
	{
		std::vector<hyprfile::RuntimeDumpFile::ModuleRecord> modules;
		runtime_dump_file.GetModuleRecords(modules);

		for (auto& module : modules)
		{
			starter.ReserveMemory(uintptr_t(module.imagebase), module.imagesize, PAGE_EXECUTE_READWRITE, module.name);
		}
	}
}

