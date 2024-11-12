#include <hyprtrace/api_tracer.h>

namespace hyprtrace
{
	hypr::Loader* ApiTracer::loader_ = nullptr;
	hyprutils::LogManager ApiTracer::logman_{ "ApiTracer" };

	bool ApiTracer::should_log_ = true;
	std::unordered_map<std::string, bool> ApiTracer::filtered_modules_;
	std::unordered_map<uintptr_t, bool> ApiTracer::filtered_apis_by_address_;
	std::unordered_map<std::string, bool> ApiTracer::filtered_apis_by_name_;

	LONG NTAPI ApiTracer::ExceptionHandler(struct _EXCEPTION_POINTERS* exception)
	{
		hyprutils::LogManager& logman = GetLogManager();
		uint32_t code = exception->ExceptionRecord->ExceptionCode;
		uintptr_t address = reinterpret_cast<uintptr_t>(exception->ExceptionRecord->ExceptionAddress);

#ifdef _WIN64
		uintptr_t return_address = *reinterpret_cast<uintptr_t*>(exception->ContextRecord->Rsp);
#else
		uintptr_t return_address = *reinterpret_cast<uintptr_t*>(exception->ContextRecord->Esp);
#endif

		if (code == EXCEPTION_PRIV_INSTRUCTION)
		{
			std::shared_ptr<const hypr::RuntimeDump::ProcRecord> proc = loader_->GetRuntimeDump().FindProcRecord(address);
			
			if (proc)
			{
				std::shared_ptr<hypr::RuntimeDump::ModuleRecord> module = proc->module.lock();

				bool filtered = false;
				// filtering by module
				{
					auto it = filtered_modules_.find(module->name);
					if (it != filtered_modules_.end())
						filtered = true;
				}
				// filtering by address
				{
					auto it = filtered_apis_by_address_.find(address);
					if (it != filtered_apis_by_address_.end())
						filtered = true;
				}
				// filtering by name
				{
					auto it = filtered_apis_by_name_.find(module->name + "!" + proc->name);
					if (it != filtered_apis_by_name_.end())
						filtered = true;
				}

				if (should_log_ && !filtered)
					logman.Log("{:X} -> {:X} {}!{} (return to {:X})", address, proc->new_address, module->name, proc->name, return_address);
		
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

	hyprutils::LogManager& ApiTracer::GetLogManager()
	{
		return logman_;
	}

	bool ApiTracer::Intialize(hypr::Loader* loader)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (loader == nullptr)
		{
			logman.Error("the parameter \"loader\" your passed in is nullptr");
			return false;
		}

		if (loader_ != nullptr)
		{
			logman.Error("api tracer is already intiialized");
			return false;
		}

		loader_ = loader;

		hypr::RuntimeDump& dump = loader->GetRuntimeDump();
		const std::vector<std::shared_ptr<hypr::RuntimeDump::ModuleRecord>>& modules = dump.GetModuleRecords();

		if (modules.empty())
		{
			logman.Log("no module loaded in runtime dump, have you ever loaded a runtime dump file?");
			return false;
		}

		for (auto& module : modules)
		{
			MEMORY_BASIC_INFORMATION mbi{};

			if (!VirtualQuery(reinterpret_cast<LPCVOID>(module->imagebase), &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
			{
				logman.Error("memory region for {} is invalid", module->name);
				return false;
			}

			if (mbi.RegionSize != module->imagesize)
			{
				logman.Error("memory region for {} is invalid", module->name);
				return false;
			}

			// hlt
			memset(reinterpret_cast<void*>(module->imagebase), 0xF4, module->imagesize);
		}

		// exception handler
		AddVectoredExceptionHandler(TRUE, ExceptionHandler);

		return true;
	}

	bool ApiTracer::AddFilteringModule(const std::string& name)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (loader_ == nullptr)
		{
			logman.Error("api tracer is not initialized");
			return false;
		}

		auto it = filtered_modules_.find(name);
		if (it != filtered_modules_.end())
		{
			logman.Warn("module {} is already being filtered", name);
			return false;
		}
		logman.Log("added filtering module {}", name);
		filtered_modules_.insert({ name ,true });
		return true;
	}

	bool ApiTracer::RemoveFilteringModule(const std::string& name)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (loader_ == nullptr)
		{
			logman.Error("api tracer is not initialized");
			return false;
		}

		auto it = filtered_modules_.find(name);
		if (it == filtered_modules_.end())
		{
			logman.Warn("module {} is not being filterd", name);
			return false;
		}

		logman.Log("removed filtering module {}", name);
		filtered_modules_.erase(it);
		return true;
	}

	bool ApiTracer::AddFilteringApi(uintptr_t address)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (loader_ == nullptr)
		{
			logman.Error("api tracer is not initialized");
			return false;
		}

		auto it = filtered_apis_by_address_.find(address);
		if (it != filtered_apis_by_address_.end())
		{
			logman.Warn("api {:X} is already being filtered", address);
			return false;
		}
		logman.Log("added filtering api {:X}", address);
		filtered_apis_by_address_.insert({ address ,true });
		return true;
	}

	bool ApiTracer::AddFilteringApi(const std::string& name)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (loader_ == nullptr)
		{
			logman.Error("api tracer is not initialized");
			return false;
		}

		auto it = filtered_apis_by_name_.find(name);
		if (it != filtered_apis_by_name_.end())
		{
			logman.Warn("api {} is already being filtered", name);
			return false;
		}
		logman.Log("added filtering api {}", name);
		filtered_apis_by_name_.insert({ name ,true });
		return true;
	}

	bool ApiTracer::RemoveFilteringApi(uintptr_t address) 
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (loader_ == nullptr)
		{
			logman.Error("api tracer is not initialized");
			return false;
		}

		auto it = filtered_apis_by_address_.find(address);
		if (it == filtered_apis_by_address_.end())
		{
			logman.Warn("api {:X} is not being filterd", address);
			return false;
		}

		logman.Log("removed filtering api {:X}", address);
		filtered_apis_by_address_.erase(it);
		return true;
	}

	bool ApiTracer::RemoveFilteringApi(const std::string& name)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (loader_ == nullptr)
		{
			logman.Error("api tracer is not initialized");
			return false;
		}

		auto it = filtered_apis_by_name_.find(name);
		if (it == filtered_apis_by_name_.end())
		{
			logman.Warn("api {} is not being filterd", name);
			return false;
		}

		logman.Log("removed filtering api {}", name);
		filtered_apis_by_name_.erase(it);
		return true;
	}

	void ApiTracer::EnableTraceLogging() { should_log_ = true; }
	void ApiTracer::DisableTraceLogging() { should_log_ = false; }
}