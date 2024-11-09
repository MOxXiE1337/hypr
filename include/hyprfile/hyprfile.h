#pragma once

#include <string>
#include <vector>
#include <memory>

namespace hyprfile
{
	class FileType
	{
	protected:
		std::shared_ptr<uint8_t[]> buffer_;
		size_t size_;

	private:
		virtual bool CheckValidity() = 0;
	public:
		FileType() = default;

		bool IsLoaded() { return buffer_ ? true : false; }
		bool LoadFromFile(const std::string& path);
		bool LoadFromMemory(const void* data, size_t size);
		bool LoadFromResource(uint32_t id, const std::string& type);
	};
}

