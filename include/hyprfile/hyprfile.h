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
		size_t size_{ 0 };

	private:
		virtual bool CheckValidity() = 0;
	public:
		FileType() = default;

		bool IsLoaded() { return buffer_ ? true : false; }
		bool LoadFromFile(const std::string& path);
		bool LoadFromMemory(const void* data, size_t size);
	};
}

