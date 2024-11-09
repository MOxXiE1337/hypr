#pragma once
#include "../hypr.h"
#include "../loader_component.h"

namespace hypr
{
	class RuntimeDump : public LoaderComponent
	{
	public:
		struct ModuleRecord;

		struct ProcRecord
		{
			std::shared_ptr<ModuleRecord> module;
			uint32_t ordinal;
			uintptr_t new_address; // new address by GetProcAddress
			std::string name;
		};

		struct ModuleRecord
		{
			std::string name;
			uintptr_t imagebase;
			size_t    imagesize;
			std::vector<std::shared_ptr<ProcRecord>> procs;

			ModuleRecord() : name(), imagebase(0), imagesize(0), procs() {}
		};
	private:
		std::vector<std::shared_ptr<ModuleRecord>> modules_;
		std::unordered_map<segaddr_t, std::shared_ptr<ProcRecord>>  gprocs_;
	public:
		RuntimeDump() = delete;
		RuntimeDump(Loader* loader);

		const std::vector<std::shared_ptr<ModuleRecord>>& GetModuleRecords() { return modules_; }
		std::unordered_map<segaddr_t, std::shared_ptr<ProcRecord>>& GetAllProcRecords() { return gprocs_; }

		std::shared_ptr<const RuntimeDump::ProcRecord> FindProcRecord(segaddr_t address);

		// .hdmp file 
		bool LoadRuntimeDumpFileFromMemory(const void* data, size_t size);
		bool LoadRuntimeDumpFileFromFile(const std::string& path);
		bool LoadRuntimeDumpFileFromResource(uint32_t id, const std::string& type);
	};
}
