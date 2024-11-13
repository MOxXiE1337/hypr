#pragma once
#include "../hypr.h"
#include "../loader_component.h"

#include <hyprutils/search.h>

namespace hypr
{
	class RuntimeDump : public LoaderComponent
	{
	public:
		struct ModuleRecord;

		struct ProcRecord
		{
			std::weak_ptr<ModuleRecord> module;
			uint32_t ordinal;
			uintptr_t address;
			uintptr_t new_address; // new address by GetProcAddress
			std::string name;
			
			bool LoadProc();
		};

		struct ModuleRecord
		{
			std::string name;
			uintptr_t imagebase;
			size_t    imagesize;
			std::vector<std::shared_ptr<ProcRecord>> procs;
			hyprutils::HSearch<std::string, ProcRecord> procs_name_search;

			ModuleRecord() : name(), imagebase(0), imagesize(0), procs() {}
		};
	private:
		std::vector<std::shared_ptr<ModuleRecord>> modules_;
		std::vector<std::shared_ptr<ProcRecord>> procs_;
	
		hyprutils::BSearch<AddressRange, ModuleRecord> modules_address_search_;
		hyprutils::HSearch<std::string, ModuleRecord> modules_name_search_;
		hyprutils::HSearch<segaddr_t, ProcRecord> procs_search_;
	public:
		RuntimeDump() = delete;
		RuntimeDump(Loader* loader);

		std::vector<std::shared_ptr<ModuleRecord>>& GetModuleRecords() { return modules_; }
		std::vector<std::shared_ptr<ProcRecord>>& GetProcRecords() { return procs_; }
		
		std::shared_ptr<RuntimeDump::ModuleRecord> FindModuleRecord(segaddr_t address);
		std::shared_ptr<RuntimeDump::ModuleRecord> FindModuleRecord(const std::string& name); // slow
		std::shared_ptr<RuntimeDump::ProcRecord> FindProcRecord(segaddr_t address);

		// .hdmp file 
		bool LoadRuntimeDumpFileFromMemory(const void* data, size_t size);
		bool LoadRuntimeDumpFileFromFile(const std::string& path);
	};
}
