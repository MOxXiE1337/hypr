#include <hypr/components/segment_mapper.h>

#include <hyprutils/file.h>

namespace hypr
{
	bool SegmentMapper::MapSegment(std::shared_ptr<Segment> segment)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (mode_ == SegmentMapperMode::kUnknown)
		{
			logman.Error("segment mapper mode is not set");
			return false;
		}

		// already mapped?
		if (segment->mapped_address != 0)
		{
			logman.Error("seg{} is already mapped", segment->ordinal);
			return false;
		}

		// map to fixed address
		if (mode_ == SegmentMapperMode::kStatic)
		{
			MEMORY_BASIC_INFORMATION mbi{};
			if (!VirtualQuery(reinterpret_cast<LPCVOID>(segment->address), &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
			{
				logman.Error("failed to query memory {:X} while mapping seg{}", segment->address, segment->ordinal);
				return false;
			}
			
			if (mbi.RegionSize < segment->vsize)
			{
				logman.Error("memory reserverd for seg{} {:X} is invalid", segment->ordinal, segment->address);
				return false;
			}

			// copy data
			memcpy(reinterpret_cast<void*>(segment->address), segment->data.get(), segment->rsize);
			segment->mapped_address = segment->address;
		}

		// map to dynamic address
		if (mode_ == SegmentMapperMode::kDynamic)
		{
			void* address = VirtualAlloc(nullptr, segment->vsize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

			if (address == nullptr)
			{
				logman.Error("failed to allocate memory while mapping seg{}", segment->ordinal);
				return false;
			}

			// copy data
			memcpy(address, segment->data.get(), segment->rsize);
			segment->mapped_address = reinterpret_cast<uintptr_t>(address);
		}

		logman.Log("mapped seg{} from {:X} to {:X}", segment->ordinal, segment->address, segment->mapped_address);

		return true;
	}

	SegmentMapper::SegmentMapper(Loader* loader) : LoaderComponent(loader, "SegmentMapper"),
		mode_(SegmentMapperMode::kUnknown),
		base_address_(0)
	{
	}

	std::shared_ptr<const SegmentMapper::Segment> SegmentMapper::FindSegmentBySegmentAddress(segaddr_t address)
	{
		std::shared_ptr<const Segment> segment = segments_segaddr_search_.Find(address);
		return segment;
	}

	std::shared_ptr<const SegmentMapper::Segment> SegmentMapper::FindSegmentByAddress(uintptr_t address)
	{
		std::shared_ptr<Segment> segment = segments_mapaddr_search_.Find({ address });
		return segment;
	}

	bool SegmentMapper::IsSegmentAddressInSegments(segaddr_t address)
	{
		std::shared_ptr<const SegmentMapper::Segment> segment = FindSegmentBySegmentAddress(address);
		return segment ? true : false;
	}

	bool SegmentMapper::IsAddressInSegments(uintptr_t address)
	{
		std::shared_ptr<const SegmentMapper::Segment> segment = FindSegmentByAddress(address);
		return segment ? true : false;
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

		std::shared_ptr<Segment> seg = std::make_shared<Segment>(
			segment.ordinal,
			segment.address,
			0,
			segment.vsize,
			segment.rsize,
			data);

		segments_.push_back(seg);
		segments_segaddr_search_.AddElement({ segment.address, segment.vsize }, seg);

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

	bool SegmentMapper::MapSegments()
	{
		for (auto& segment : segments_)
		{
			if (!MapSegment(segment))
				return false;
		}
		return true;
	}

	uintptr_t SegmentMapper::TranslateAddress(segaddr_t address)
	{
		std::shared_ptr<const Segment> segment = FindSegmentBySegmentAddress(address);

		if (!segment)
			return 0;

		// not mapped yet
		if (segment->mapped_address == 0)
			return 0;

		return address - segment->address + segment->mapped_address;
	}

	uintptr_t SegmentMapper::TranslateOffset(ptrdiff_t offset)
	{
		return TranslateAddress(base_address_ + offset);
	}

}

