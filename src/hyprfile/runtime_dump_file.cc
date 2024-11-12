#include <hyprfile/runtime_dump_file.h>


namespace hyprfile
{
	bool RuntimeDumpFile::CheckValidity()
	{
		if (!buffer_)
			return false;

		if (size_ < sizeof(hdmpfile_t) + sizeof(hdmpmod_t) + sizeof(hdmpproc_t))
			return false;

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hdmpfile_t* header = reinterpret_cast<const hdmpfile_t*>(data);

		if (header->magic != kRuntimeDumpFileMagicNumber)
			return false;

		if (header->module_num == 0 || header->module_names == 0 || header->procs == 0 || header->proc_names == 0)
			return false;

		return true;
	}

	void RuntimeDumpFile::GetModuleRecords(std::vector<ModuleRecord>& out)
	{
		if (!buffer_)
			return;

		out.clear();

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hdmpfile_t* header = reinterpret_cast<const hdmpfile_t*>(data);
		const hdmpmod_t* mod = reinterpret_cast<const hdmpmod_t*>(data + sizeof(hdmpfile_t));
		const char* module_names = reinterpret_cast<const char*>(data + header->module_names);
		const char* procs = reinterpret_cast<const char*>(data + header->procs);

		for (size_t i = 0; i < header->module_num; i++)
		{
			out.push_back(
				{
					module_names + mod[i].name,
					mod[i].imagebase,
					mod[i].imagesize,
					mod[i].proc_num,
					reinterpret_cast<const hdmpproc_t*>(procs + mod[i].procs)
				}
			);
		}
	}

	void RuntimeDumpFile::GetProcRecords(const ModuleRecord& module, std::vector<ProcRecord>& out)
	{
		if (!buffer_)
			return;

		out.clear();

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hdmpfile_t* header = reinterpret_cast<const hdmpfile_t*>(data);
		const char* proc_names = reinterpret_cast<const char*>(data + header->proc_names);

		for (size_t i = 0; i < module.proc_num; i++)
		{
			const hdmpproc_t& proc = module.procs[i];
			if(proc.name == 0)
				out.push_back({ proc.ordinal,"", proc.address});
			else
				out.push_back({ proc.ordinal,proc_names + proc.name, proc.address });
		}

	}
}

