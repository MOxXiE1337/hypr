#pragma once
#include "hyprfile.h"

namespace hyprfile
{
	constexpr uint32_t kSegmentsFileMagicNumber = 'GESH'; // HSEG

#pragma pack(1)
	struct hsegfile_t
	{
		uint32_t magic;
		uint64_t base_address; // optional
		uint32_t segment_num;
	};

	struct hsegseg_t
	{
		uint32_t ordinal;
		uint64_t address;
		uint32_t vsize; // virtual size
		uint32_t rsize; // raw size
		uint32_t data; // offset
	};
#pragma pack()

	class SegmentsFile
	{
	public:
		struct Segment
		{
			uint32_t  ordinal;
			uintptr_t address;
			size_t    vsize;
			size_t    rsize;
			const uint8_t* data;
		};
	private:
		std::shared_ptr<uint8_t[]> buffer_;
		size_t                     size_;

		bool CheckValidity();
	public:
		bool IsLoaded() { return buffer_ ? true : false; }
		bool Load(const std::string& path);
		bool Load(const void* data, size_t size);

		void GetSegments(std::vector<Segment>& out);
	};
}
