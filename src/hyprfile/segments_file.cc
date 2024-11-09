#include <hyprfile/segments_file.h>
#include <hyprutils/file.h>

namespace hyprfile
{
    bool SegmentsFile::Load(const std::string& path)
    {
		if (buffer_)
			return false;

		buffer_ = hyprutils::ReadFileToMemory(path, &size_);

		if (!buffer_)
			return false;

		return CheckValidity();
    }

	bool SegmentsFile::Load(const void* data, size_t size)
	{
		if (buffer_)
			return false;

		if (size < sizeof(hsegfile_t) + sizeof(hsegseg_t) + 0x100)
			return false;

		buffer_ = std::make_shared<uint8_t[]>(size);
		size_ = size;

		memcpy(buffer_.get(), data, size);

		return CheckValidity();
	}

	bool SegmentsFile::CheckValidity()
	{
		if (!buffer_)
			return false;

		const char* data = reinterpret_cast<const char*>(buffer_.get());
		const hsegfile_t* header = reinterpret_cast<const hsegfile_t*>(data);

		if (header->magic != kSegmentsFileMagicNumber)
			return false;

		if (header->segment_num == 0)
			return false;

		return true;
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
