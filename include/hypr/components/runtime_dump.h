#pragma once
#include "../hypr.h"
#include "../loader_component.h"

namespace hypr
{
	class RuntimeDump : public LoaderComponent
	{
	public:
		struct ProcRecord
		{
			uint32_t ordinal;
			uintptr_t new_address; // new address by GetProcAddress
			std::string name;
		};

		struct ModuleRecord
		{
			std::string name;
			uintptr_t imagebase;
			size_t    imagesize;
			std::vector<ProcRecord> procs;
		};
	private:
		std::vector<std::shared_ptr<ModuleRecord>> modules_;
	public:
		RuntimeDump() = delete;
		RuntimeDump(Loader* loader);

		const std::vector<std::shared_ptr<ModuleRecord>>& GetModuleRecords() { return modules_; }

		const ProcRecord* FindProcRecord(uintptr_t old_address);

		// .hdmp file 
		bool LoadRuntimeDumpFileFromMemory(const void* data, size_t size);
		bool LoadRuntimeDumpFileFromFile(const std::string& path);
		bool LoadRuntimeDumpFileFromResource(uint32_t id, const std::string& type);
	};
}
