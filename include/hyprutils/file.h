#pragma once

#include <memory>
#include <fstream>

namespace hyprutils
{
	static std::shared_ptr<uint8_t[]> ReadFileToMemory(const std::string& path, size_t* readed_size = nullptr)
	{
		std::ifstream file;
		size_t        size = 0;

		file.open(path, std::ios::binary);

		if (!file.is_open())
			return {};

		file.seekg(0, std::ios::end);
		size = file.tellg();
		file.seekg(0);

		std::shared_ptr<uint8_t[]> buffer = std::make_shared<uint8_t[]>(size);

		if (!buffer)
			return {};

		file.read(reinterpret_cast<char*>(buffer.get()), size);
		file.close();

		if (readed_size)
			*readed_size = size;

		return buffer;
	}
}
