#pragma once
#include "hyprfile.h"

namespace hyprfile
{
	constexpr uint32_t kRuntimeDumpFileMagicNumber = 'PMDH'; // HDMP

#pragma pack(1)
	struct hdmpmod_t
	{
		uint32_t name; // offset from module_names
		uint64_t imagebase;
		uint32_t imagesize;
		uint32_t proc_num;
		uint32_t procs; // offset
	};

	struct hdmpproc_t
	{
		uint32_t ordinal;
		uint32_t name; // offset from proc_names
		uint64_t address;
	};

	struct hdmpfile_t
	{
		uint32_t magic;
		uint32_t module_num;
		uint32_t module_names; // offset, data compressed with zlib
		uint32_t proc_names; // offset, data compressed with zlib
		uint32_t procs; // offset
	};

#pragma pack()

	class RuntimeDumpFile : public FileType
	{
	public:
		struct ProcRecord
		{
			uint32_t ordinal;
			const char* name;
			uint64_t address;
		};

		struct ModuleRecord
		{
			const char* name;
			uint64_t imagebase;
			size_t imagesize;
			size_t proc_num;
			const hdmpproc_t* procs;
		};

	private:
		bool CheckValidity() override;
	public:
		RuntimeDumpFile() : FileType() {}

		void GetModuleRecords(std::vector<ModuleRecord>& out);
		void GetProcRecords(const ModuleRecord& module, std::vector<ProcRecord>& out);
	};
}
