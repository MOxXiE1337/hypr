#pragma once
#include "../hypr.h"

namespace hypr
{
	constexpr uint32_t kHDMPFileMagicNumber = 'PMDH';

#pragma pack(1)
	struct hdmpmod_t
	{
		uint32_t name; // offset from module_names
		uint64_t imagebase;
		uint32_t imagesize;
		uint32_t proc_number;
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
		uint32_t module_number;
		uint32_t module_names; // offset, data compressed with zlib
		uint32_t proc_names; // offset, data compressed with zlib
		uint32_t procs; // offset
	};

#pragma pack()

	class hdmp
	{
	public:
		struct hdmp_proc
		{
			uint32_t ordinal;
			const char* name;
			uint64_t address;
		};

		struct hdmp_module
		{
			const char* name;
			uint64_t imagebase;
			size_t imagesize;
			size_t proc_number;
			const hdmpproc_t* procs;
		};

	private:

		std::shared_ptr<uint8_t[]> buffer_;
		size_t size_;

		size_t module_num;

		bool Parse();
	public:
		bool Load(const char* path);
		bool Load(const void* data, size_t size);

		void GetModules(std::vector<hdmp_module>& out);
		void GetProcs(const hdmp_module& module, std::vector<hdmp_proc>& out);
	};
}
