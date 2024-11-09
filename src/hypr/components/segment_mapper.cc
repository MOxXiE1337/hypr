#include <hypr/components/segment_mapper.h>

#include <hyprutils/file.h>
#include <hyprutils/resource.h>

namespace hypr
{
	SegmentMapper::SegmentMapper(Loader* loader) : LoaderComponent(loader, "SegmentMapper"),
		mode_(SegmentMapperMode::kUnknown),
		base_address_(0),
		segments_()
	{
	}

	bool SegmentMapper::LoadNativeSegment(const hyprfile::SegmentsFile::Segment& segment)
	{
		hyprutils::LogManager& logman = GetLogManager();
		
		std::shared_ptr<uint8_t[]> data = std::make_shared<uint8_t[]>(segment.rsize);

		if (!data)
		{
			logman.Error("failed to allocate memory while loading segment {}", segment.ordinal);
			return false;
		}

		memcpy(data.get(), segment.data, segment.rsize);

		segments_.push_back(
			{
				segment.ordinal,
				segment.address,
				segment.vsize,
				segment.rsize,
				data
			});

		return true;
	}

	bool SegmentMapper::LoadSegmentsFileFromMemory(const void* data, size_t size)
	{
		hyprutils::LogManager& logman = GetLogManager();
		hyprfile::SegmentsFile segments_file{};

		if (!segments_file.LoadFromMemory(data, size))
		{
			logman.Error("failed to parse segments file");
			return false;
		}

		SetBaseAddress(segments_file.GetBaseAddress());

		std::vector<hyprfile::SegmentsFile::Segment> segments;
		segments_file.GetSegments(segments);

		for (auto& segment : segments)
		{
			if (!LoadNativeSegment(segment))
				return false;
		}

		return true;
	}

	bool SegmentMapper::LoadSegmentsFileFromFile(const std::string& path)
	{
		hyprutils::LogManager& logman = GetLogManager();

		size_t size = 0;
		std::shared_ptr<uint8_t[]> buffer = hyprutils::ReadFileToMemory(path, &size);

		if (!buffer || size == 0)
		{
			logman.Error("failed to read segments file \"{}\"", path);
			return false;
		}

		return LoadSegmentsFileFromMemory(buffer.get(), size);
	}

	bool SegmentMapper::LoadSegmentsFileFromResource(uint32_t id, const std::string& type)
	{
		hyprutils::LogManager& logman = GetLogManager();

		size_t size = 0;
		std::shared_ptr<uint8_t[]> buffer = hyprutils::ReadResourceToMemory(id, type, &size);

		if (!buffer || size == 0)
		{
			logman.Error("failed to read segments file from resource {}", id);
			return false;
		}

		return LoadSegmentsFileFromMemory(buffer.get(), size);
	}
}

