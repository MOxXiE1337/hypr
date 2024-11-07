#include <hypr/internal/hdmpfile.h>

namespace hypr
{
	bool hdmp::load(const char* path)
	{
		if (buffer_)
			return false;

		std::ifstream file;
		file.open(path, std::ios::binary);

		if (!file.is_open())
			return false;

		file.seekg(0, std::ios::end);
		size_t file_size = file.tellg();
		file.seekg(0);

		if (file_size < sizeof(hdmpfile_t) + sizeof(hdmpmod_t) + sizeof(hdmpproc_t))
			return false;

		buffer_ = std::make_shared<uint8_t[]>(file_size);
		size_ = file_size;

		file.read(reinterpret_cast<char*>(buffer_.get()), size_);
		file.close();

		return parse();
	}

	bool hdmp::load(const void* data, size_t size)
	{
		if (buffer_)
			return false;

		if (size < sizeof(hdmpfile_t) + sizeof(hdmpmod_t) + sizeof(hdmpproc_t))
			return false;

		buffer_ = std::make_shared<uint8_t[]>(size);
		size_ = size;

		memcpy(buffer_.get(), data, size);

		return parse();
	}

	bool hdmp::parse()
	{
		if (!buffer_)
			return false;

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hdmpfile_t* header = reinterpret_cast<const hdmpfile_t*>(data);

		if (header->magic != kHDMPFileMagicNumber)
			return false;

		if (header->module_number == 0 || header->module_names == 0 || header->procs == 0 || header->proc_names == 0)
			return false;

		return true;
	}

	void hdmp::get_modules(std::vector<hdmp_module>& out)
	{
		if (!buffer_)
			return;

		out.clear();

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hdmpfile_t* header = reinterpret_cast<const hdmpfile_t*>(data);
		const hdmpmod_t* mod = reinterpret_cast<const hdmpmod_t*>(data + sizeof(hdmpfile_t));
		const char* module_names = reinterpret_cast<const char*>(data + header->module_names);
		const char* procs = reinterpret_cast<const char*>(data + header->procs);

		for (size_t i = 0; i < header->module_number; i++)
		{
			out.push_back(
				hdmp_module{
					module_names + mod[i].name,
					mod[i].imagebase,
					mod[i].imagesize,
					mod[i].proc_number,
					reinterpret_cast<const hdmpproc_t*>(procs + mod[i].procs)
				}
			);
		}
	}

	void hdmp::get_procs(const hdmp_module& module, std::vector<hdmp_proc>& out)
	{
		if (!buffer_)
			return;

		out.clear();

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hdmpfile_t* header = reinterpret_cast<const hdmpfile_t*>(data);
		const char* proc_names = reinterpret_cast<const char*>(data + header->proc_names);

		for (size_t i = 0; i < module.proc_number; i++)
		{
			const hdmpproc_t& proc = module.procs[i];
			if(proc.name == 0)
				out.push_back({ proc.ordinal,"NONE", proc.address});
			else
				out.push_back({ proc.ordinal,proc_names + proc.name, proc.address });
		}

	}
}

