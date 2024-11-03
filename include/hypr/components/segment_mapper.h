#pragma once
#include "../loader_component.h"

namespace hypr
{
	constexpr size_t kSegmentMapperMaxTranslateCacheNumber = 10;

	enum class SegmentMapperMode
	{
		kUnknown,
		kStatic,
		kDynamic
	};

	struct Segment
	{
		uint16_t ordinal;
		segaddr_t segment_address;
		uintptr_t mapped_address;
		size_t    size;
		const uint8_t* data;
	};

	class SegmentMapper : public LoaderComponent
	{
	private:
		bool      loaded_;
		segaddr_t imagebase_;
		std::unordered_map<uint16_t, Segment> segments_;

		std::list<uint16_t> translate_cache_; // max size == 2
	public:
		SegmentMapper() = delete;
		SegmentMapper(Loader* loader);

		segaddr_t GetImagebase() { return imagebase_; }
		void SegImagebase(segaddr_t imagebase);

		void LoadNativeSegment(const Segment& segment);
		void LoadNativeSegments(const std::vector<Segment>& segments);
		void LoadSegmentsFileFromMemory(const void* data, size_t size);
		void LoadSegmentsFileFromFile(const std::string& path);
		void LoadSegmentsFileFromResource(const std::string& name, const std::string& type);

		uintptr_t TranslateAddress(segaddr_t address); // translate segment address to mapped address
		uintptr_t TranslateOffset(ptrdiff_t offset); // equals TranslateAddress(GetImagebase() + offset)
		void      PrintTranslateCache();
	private:

		friend Loader;

		bool MapSegment(uint16_t ordinal);
		bool MapSegments();
	};
}
