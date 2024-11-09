#pragma once

#pragma warning(disable: 4200)

namespace hypr
{
	constexpr uint32_t kSegmentsFileMagicNumber = 'HSEG';

#pragma pack(1)
	struct seg_t
	{
		uint16_t ordinal;
		uint64_t address : 48;
		uint32_t size;
		uint32_t raw;
	};

	struct segs_t
	{
		uint32_t magic;
		uint16_t segs_num;
		uint64_t imagebase : 48;
		uint32_t raw_data; // offset to raw data
		seg_t    segments[];
	};

#pragma pack()
}
