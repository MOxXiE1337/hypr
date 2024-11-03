#include <hypr/components/segment_mapper.h>

#include <hypr/internal/segtype.h>

namespace hypr
{
	SegmentMapper::SegmentMapper(Loader* loader) : LoaderComponent(loader, "SegmentMapper"),
		loaded_(false),
		imagebase_(0)
	{
	}

	void SegmentMapper::SegImagebase(segaddr_t imagebase)
	{
		if (loaded_)
		{
			GetLogManager().Error(true, "imagebase can't be set after segments loaded");
			return;
		}
		imagebase_ = imagebase;
	}

	void SegmentMapper::LoadNativeSegment(const Segment& segment)
	{
		auto it = segments_.find(segment.ordinal);

		if (it != segments_.end())
			GetLogManager().Error(true, "seg{} is already loaded", segment.ordinal);

		segments_.insert({ segment.ordinal, segment });
	}

	void SegmentMapper::LoadNativeSegments(const std::vector<Segment>& segments)
	{
		for (auto& segment : segments)
			LoadNativeSegment(segment);
	}

	void SegmentMapper::LoadSegmentsFileFromMemory(const void* data, size_t size)
	{
		LogManager& logman = GetLogManager();

		if (size < sizeof(segs_t) + sizeof(seg_t) + 0x1000)
			logman.Error(true, "invalid segments file (size too small)");

		const segs_t* header = reinterpret_cast<const segs_t*>(data);
		
		if(header->magic != kSegmentsFileMagicNumber)
			logman.Error(true, "invalid segments file (wrong magic number)");

	}

	void SegmentMapper::LoadSegmentsFileFromFile(const std::string& path)
	{
	}

	void SegmentMapper::LoadSegmentsFileFromResource(const std::string& name, const std::string& type)
	{
		LogManager& logman = GetLogManager();

		HRSRC src = FindResourceA(NULL, name.c_str(), type.c_str());
		uint32_t size = 0;
		HGLOBAL global = NULL;

		if(src == NULL)
			logman.Error(true, "failed to find resource \"{}\"", name);

		size = SizeofResource(NULL, src);

		if (size == 0)
			logman.Error(true, "failed to get resource \"{}\" size", name);

		global = LoadResource(NULL, src);

		if(global == NULL)
			logman.Error(true, "failed to load resource \"{}\"", name);

		const void* data = LockResource(global);

		if(data == nullptr)
			logman.Error(true, "failed to lock resource \"{}\"", name);

		LoadSegmentsFileFromMemory(data, size);

		// anyway the process will exit if some error occurs in LoadSegmentsFileFromMemory, let system do the jobs
		if(FreeResource(global) == FALSE)
			logman.Error(true, "failed to free resource \"{}\"", name);
	}
	
	uintptr_t SegmentMapper::TranslateAddress(segaddr_t address)
	{
		LogManager& logman = GetLogManager();

		// search cache first
		for (auto& cache : translate_cache_)
		{
			auto it = segments_.find(cache);
			if (it == segments_.end())
				logman.Error(true, "translate cache is corrupted");

			Segment& seg = it->second;

			if (address >= seg.segment_address && address < seg.segment_address + seg.size)
				return seg.mapped_address + (address - seg.segment_address);
		}

		// search all segments
		for (auto& [ordinal, seg] : segments_)
		{
			if (address >= seg.segment_address && address < seg.segment_address + seg.size)
			{
				if (translate_cache_.size() >= kSegmentMapperMaxTranslateCacheNumber)
					translate_cache_.resize(kSegmentMapperMaxTranslateCacheNumber - 1);

				if (!translate_cache_.empty())
				{
					if(translate_cache_.front() != ordinal)
						translate_cache_.push_front(ordinal);
				}
				else
				{
					translate_cache_.push_front(ordinal);
				}
				return seg.mapped_address + (address - seg.segment_address);
			}
		}

		return 0;
	}

	uintptr_t SegmentMapper::TranslateOffset(ptrdiff_t offset)
	{
		return TranslateAddress(imagebase_ + offset);
	}

	void SegmentMapper::PrintTranslateCache()
	{
		LogManager& logman = GetLogManager();

		if (translate_cache_.empty())
		{
			return logman.Log("translate cache is empty");
		}

		logman.Log("translate cache view, max({})", kSegmentMapperMaxTranslateCacheNumber);
		for (auto& cache : translate_cache_)
		{
			auto it = segments_.find(cache);
			if (it == segments_.end())
				logman.Error(true, "translate cache is corrupted");

			logman.Log("seg{} {:X} -> {:X}, {:X}", cache, it->second.segment_address, it->second.mapped_address, it->second.size);
		}
	}

	bool SegmentMapper::MapSegment(uint16_t ordinal)
	{
		return true;
	}

	bool SegmentMapper::MapSegments()
	{
		for (auto& [ordinal, seg] : segments_)
		{
			if (!MapSegment(ordinal))
				return false;
		}
		return true;
	}
}