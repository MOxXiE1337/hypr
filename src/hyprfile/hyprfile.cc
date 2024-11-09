#include <hyprfile/hyprfile.h>
#include <hyprutils/file.h>
#include <hyprutils/resource.h>

namespace hyprfile
{
	bool hyprfile::FileType::LoadFromFile(const std::string& path)
	{
		if (buffer_)
			return false;

		buffer_ = hyprutils::ReadFileToMemory(path, &size_);

		if (!buffer_ || size_ == 0)
			return false;

		return CheckValidity();
	}

	bool FileType::LoadFromMemory(const void* data, size_t size)
	{
		if (buffer_)
			return false;

		buffer_ = std::make_shared<uint8_t[]>(size);
		size_ = size;

		memcpy(buffer_.get(), data, size);

		return CheckValidity();
	}

	bool FileType::LoadFromResource(uint32_t id, const std::string& type)
	{
		if (buffer_)
			return false;

		buffer_ = hyprutils::ReadResourceToMemory(id, type, &size_);

		if (!buffer_ || size_ == 0)
			return false;

		return CheckValidity();
	}
}

