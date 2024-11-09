#pragma once
#include "hyprutils.h"

namespace hyprutils
{
	static std::shared_ptr<uint8_t[]> ReadResourceToMemory(uint32_t id, const std::string& type, size_t* readed_size)
	{
		HRSRC src = FindResourceA(NULL, MAKEINTRESOURCEA(id), type.c_str());
		uint32_t size = 0;
		HGLOBAL global = NULL;

		if (src == NULL)
		{
			if (readed_size)
				*readed_size = 0;
			return {};
		}
		
		size = SizeofResource(NULL, src);

		if (size == 0)
		{
			if (readed_size)
				*readed_size = 0;
			return {};
		}

		global = LoadResource(NULL, src);

		if(global == NULL)
		{
			if (readed_size)
				*readed_size = 0;
			return {};
		}

		const void* data = LockResource(global);

		if(data == nullptr)
		{
			if (readed_size)
				*readed_size = 0;
			return {};
		}
		
		std::shared_ptr<uint8_t[]> buffer = std::make_shared<uint8_t[]>(size);

		if (!buffer)
		{
			if (readed_size)
				*readed_size = 0;
			return {};
		}

		memcpy(buffer.get(), data, size);

		FreeResource(global);

		*readed_size = size;

		return buffer;
	}
}
