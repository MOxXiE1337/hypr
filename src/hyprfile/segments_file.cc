#include <hyprfile/segments_file.h>
#include <hyprutils/file.h>
#include <hyprutils/resource.h>

namespace hyprfile
{
	bool SegmentsFile::CheckValidity()
	{
		if (!buffer_)
			return false;

		if (size_ < sizeof(hsegfile_t) + sizeof(hsegseg_t))
			return false;

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hsegfile_t* header = reinterpret_cast<const hsegfile_t*>(data);

		if (header->magic != kSegmentsFileMagicNumber)
			return false;

		if (header->segment_num == 0)
			return false;

		base_address_ = (uintptr_t)header->base_address;

		return true;
	}

	uintptr_t SegmentsFile::GetBaseAddress()
	{
		return base_address_;
	}

	void SegmentsFile::GetSegments(std::vector<Segment>& out)
	{
		if (!buffer_)
			return;

		out.clear();

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hsegfile_t* header = reinterpret_cast<const hsegfile_t*>(data);
		uint32_t          segment_num = header->segment_num;
		const hsegseg_t*  segments = reinterpret_cast<const hsegseg_t*>(data + sizeof(hsegfile_t));
		const uint8_t*       raw_data = reinterpret_cast<const uint8_t*>(data + sizeof(hsegfile_t) * segment_num);

		for (uint32_t i = 0; i < segment_num; i++)
		{
			out.push_back(
				Segment{
					segments[i].ordinal,
					uintptr_t(segments[i].address),
					segments[i].vsize,
					segments[i].rsize,
					raw_data + segments[i].data
				}
				);
		}
	}





}

