#include <hyprtrace/tracer.h> 

namespace hyprtrace
{
	hypr::Loader* Tracer::loader_ = nullptr;
	hyprutils::LogManager Tracer::logman_{ "Tracer" };

	LONG NTAPI Tracer::ExceptionHandler(struct _EXCEPTION_POINTERS* exception)
	{
		hyprutils::LogManager& logman = GetLogManager();
		uintptr_t address = reinterpret_cast<uintptr_t>(exception->ExceptionRecord->ExceptionAddress);
#ifdef _WIN64
		uintptr_t ret_address = *reinterpret_cast<uintptr_t*>(exception->ContextRecord->Rsp);
#else
		uintptr_t ret_address = *reinterpret_cast<uintptr_t*>(exception->ContextRecord->Esp);
#endif

		if (exception->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)
		{
			std::shared_ptr<const hypr::RuntimeDump::ProcRecord> proc = loader_->GetRuntimeDump().FindProcRecord(address);

			if (proc)
			{
				logman.Log("HIT: {:X} -> {:X} {}.{} (ret to {:X})", address, proc->new_address, proc->module.lock()->name, proc->name, ret_address);
#ifdef _WIN64
				exception->ContextRecord->Rip = proc->new_address;
#else
				exception->ContextRecord->Eip = proc->new_address;
#endif
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	bool Tracer::Initialize(hypr::Loader* loader)
	{
		loader_ = loader;

		hyprutils::LogManager& logman = GetLogManager();
		hypr::RuntimeDump& dump = loader->GetRuntimeDump();

		for (auto& module : dump.GetModuleRecords())
		{
			// hlt
			memset(reinterpret_cast<void*>(module->imagebase), 0xF4, module->imagesize);
		}

		AddVectoredExceptionHandler(TRUE, ExceptionHandler);

		return true;
	}	

	bool Tracer::InitializeProcessStarter(hyprocess::ProcessStarter& starter, hyprfile::RuntimeDumpFile& runtime_dump_file)
	{
		std::vector<hyprfile::RuntimeDumpFile::ModuleRecord> modules;
		runtime_dump_file.GetModuleRecords(modules);

		for (auto& module : modules)
		{
			starter.ReserveMemory(uintptr_t(module.imagebase), module.imagesize, PAGE_EXECUTE_READWRITE, module.name);
		}

		return true;
	}
}

